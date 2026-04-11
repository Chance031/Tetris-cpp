#include "Game.h"
#include "Renderer.h"

// 표준 라이브러리 유틸리티와 입출력
#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

// Windows 전용 입력 처리: _kbhit(), _getch() 사용.
// 크로스플랫폼 빌드 시 대체 필요.
#include <conio.h>

namespace
{
	// 키 입력 상수를 정의한다.
	constexpr int KeyEscape = 27;
	constexpr int KeyEnter = 13;
	constexpr int KeyBackspace = 8;
	constexpr int KeyExtendedPrefix = 224;
	constexpr int KeyExtendedPrefixAlt = 0;
	constexpr int KeyArrowLeft = 75;
	constexpr int KeyArrowRight = 77;
	constexpr int KeyArrowDown = 80;
}

Game::Game() : m_randomEngine(std::random_device{}())
{
}

void Game::Initialize()
{
	LoadHighScores();
	TransitionTo(GameState::Title);
}

void Game::Run()
{
	// 50ms 프레임 지연한다. 각 루프의 사이 50ms씩 기간을 두어, 너무 빠르게 돌아가지 않도록 한다.
	constexpr auto FrameDelay = std::chrono::milliseconds(50);

	// [2J: 화면 전체 지우기 / [H: 커서를 홈 위치(보통 좌상단)로 이동 / [?25l: 커서 숨기기
	std::cout << "\x1B[2J\x1B[H\x1B[?25l";

	while (m_state != GameState::Exit)
	{
		if (m_state == GameState::Title)
		{
			Render();
			HandleTitleInput();
			continue;
		}

		while (m_state == GameState::Playing)
		{
			HandleInput();

			if (m_state != GameState::Playing)
				break;

			Update();
			Render();

			std::this_thread::sleep_for(FrameDelay);
		}

		Render();

		if (m_state == GameState::Paused)
			HandlePausedInput();
		else if (m_state == GameState::GameOver)
			HandleGameOverInput();
		else if (m_state != GameState::Exit)
			TransitionTo(GameState::Exit);
	}

	Render();
	// [?25h: 커서 다시 보이게 하기
	std::cout << "\x1B[?25h" << std::flush;
}

// 새 게임을 시작하기 위해 필요한 모든 진행 상태를 초기값으로 되돌린다.
// 첫 피스를 준비한 뒤 게임 상태를 Playing으로 전환하는 함수이다.
void Game::StartNewSession()
{
	m_board.Reset();
	m_score = 0;
	m_level = 1;
	m_totalLines = 0;
	m_combo = -1;
	m_isBackToBackActive = false;
	m_lastClearMessage.clear();
	m_needsHighScoreName = false;
	m_isLockRequired = false;
	m_lastMoveWasRotation = false;
	ResetLockDelay();
	m_hasHoldPiece = false;
	m_canHold = true;
	TransitionTo(GameState::Playing);

	m_fallInterval = std::chrono::milliseconds(InitialFallIntervalMs);
	m_lastFallTime = std::chrono::steady_clock::now();

	m_nextPiece = Tetromino(CreateRandomTetrominoType());
	SpawnNextPiece();
}

// 게임 상태를 바꾸는 함수
void Game::TransitionTo(GameState newState)
{
	m_state = newState;
}

void Game::HandleInput()
{
	if (!_kbhit())
		return;

	int key = _getch();

	if (key == KeyEscape || key == 'q' || key == 'Q')
	{
		TransitionTo(GameState::Exit);
		return;
	}

	if (key == 'p' || key == 'P')
	{
		TransitionTo(GameState::Paused);
		return;
	}

	if (key == KeyExtendedPrefix || key == KeyExtendedPrefixAlt)
	{
		key = _getch();

		switch (key)
		{
		case KeyArrowLeft:
			if (TryMoveCurrentPiece(-1, 0, false))
				m_lastMoveWasRotation = false;
			break;
		case KeyArrowRight:
			if (TryMoveCurrentPiece(1, 0, false))
				m_lastMoveWasRotation = false;
			break;
		case KeyArrowDown:
			if (TryMoveCurrentPiece(0, 1, true))
				m_score += SoftDropScorePerCell;
			break;
		}

		return;
	}

	if (key == 'z' || key == 'Z')
	{
		TryRotateCurrentPiece(RotationDirection::Clockwise);
	}
	else if (key == 'x' || key == 'X')
	{
		TryRotateCurrentPiece(RotationDirection::CounterClockwise);
	}
	else if (key == ' ')
	{
		HardDropCurrentPiece();
	}
	else if (key == 'c' || key == 'C')
	{
		HoldCurrentPiece();
	}
}

void Game::HandleTitleInput()
{
	while (m_state == GameState::Title)
	{
		const int key = _getch();

		if (key == KeyEnter || key == ' ')
		{
			StartNewSession();
		}
		else if (key == KeyEscape || key == 'q' || key == 'Q')
		{
			TransitionTo(GameState::Exit);
		}
	}
}

void Game::HandlePausedInput()
{
	while (m_state == GameState::Paused)
	{
		const int key = _getch();

		if (key == 'p' || key == 'P')
		{
			TransitionTo(GameState::Playing);
			m_lastFallTime = std::chrono::steady_clock::now();

			if (m_isTouchingGround)
				m_lockStartTime = std::chrono::steady_clock::now();
		}
		else if (key == KeyEscape || key == 'q' || key == 'Q')
		{
			TransitionTo(GameState::Exit);
		}
	}
}

void Game::HandleGameOverInput()
{
	if (m_needsHighScoreName)
	{
		PromptAndSaveHighScore();
		Render();
	}

	while (m_state == GameState::GameOver)
	{
		const int key = _getch();

		if (key == 'r' || key == 'R')
		{
			StartNewSession();
			Render();
		}
		else if (key == KeyEscape || key == 'q' || key == 'Q')
		{
			TransitionTo(GameState::Exit);
		}
	}
}

void Game::Update()
{
	if (m_isLockRequired)
	{
		ProcessLockAndResolve();
		return;
	}

	const auto now = std::chrono::steady_clock::now();

	if (m_isTouchingGround && now - m_lockStartTime >= m_lockDelay)
	{
		m_isLockRequired = true;
		ProcessLockAndResolve();
		return;
	}

	if (now - m_lastFallTime < m_fallInterval)
		return;

	m_lastFallTime = now;
	TryMoveCurrentPiece(0, 1, true);
}

void Game::Render()
{
	if (m_state == GameState::Title)
	{
		std::ostringstream titleFrame;
		titleFrame << "\x1B[2J\x1B[H";
		titleFrame << "TETRIS\n\n";
		titleFrame << "Press Enter or Space to Start\n";
		titleFrame << "Press Q or Esc to Quit\n";
		RenderHighScores(titleFrame);
		std::cout << titleFrame.str() << std::flush;
		return;
	}

	if (m_state == GameState::Exit)
	{
		std::cout << "\x1B[2J\x1B[HQuit\n" << std::flush;
		return;
	}

	const auto currentBlocks = m_currentPiece.GetBlockLocations();
	const auto nextBlocks = m_nextPiece.GetBlockLocations();
	const auto ghostBlocks = GetGhostPiece().GetBlockLocations();
	const auto holdBlocks = m_holdPiece.GetBlockLocations();
	std::ostringstream frame;

	frame << "\x1B[H\x1B[J";
	frame << "Score: " << m_score
		<< "  Level: " << m_level
		<< "  Lines: " << m_totalLines
		<< "  Combo: " << std::max(m_combo, 0)
		<< "  B2B: " << (m_isBackToBackActive ? "On" : "Off");

	if (!m_lastClearMessage.empty())
		frame << "  Last: " << m_lastClearMessage;

	frame << '\n';

	for (int y = 0; y < Board::Height; ++y)
	{
		Renderer::RenderBoardRow(frame, m_board, currentBlocks, ghostBlocks, y);
		Renderer::RenderSidePanelRow(frame, nextBlocks, holdBlocks, m_hasHoldPiece, y);
		frame << '\n';
	}

	frame << "\nControls: Left/Right = Move, Down = Soft Drop, Z/X = Rotate, Space = Hard Drop, C = Hold, P = Pause, Q/Esc = Quit\n";

	Renderer::RenderStatusMessage(frame, m_state);

	if (m_state == GameState::GameOver)
		RenderHighScores(frame);

	std::cout << frame.str() << std::flush;
}

void Game::PromptAndSaveHighScore()
{
	m_needsHighScoreName = false;

	std::string name;
	std::cout << "\nNew score: " << m_score << "\n";
	std::cout << "Enter your name: " << std::flush;

	while (true)
	{
		const int key = _getch();

		if (key == KeyEnter)
			break;

		if (key == KeyBackspace)
		{
			if (!name.empty())
			{
				name.pop_back();
				std::cout << "\b \b" << std::flush;
			}

			continue;
		}

		if (key <= 32 || key > 126)
			continue;

		if (static_cast<int>(name.size()) >= MaxPlayerNameLength)
			continue;

		name.push_back(static_cast<char>(key));
		std::cout << static_cast<char>(key) << std::flush;
	}

	if (name.empty())
		name = "PLAYER";

	AddHighScore(name, m_score);
}

void Game::AddHighScore(const std::string& name, int score)
{
	m_highScores.push_back({ name, score });

	std::sort(m_highScores.begin(), m_highScores.end(), [](const HighScoreEntry& lhs, const HighScoreEntry& rhs)
	{
		return lhs.score > rhs.score;
	});

	if (static_cast<int>(m_highScores.size()) > MaxHighScoreCount)
		m_highScores.resize(MaxHighScoreCount);

	SaveHighScores();
}

void Game::SaveHighScores() const
{
	std::ofstream file("scores.txt");

	for (const HighScoreEntry& entry : m_highScores)
		file << entry.name << ' ' << entry.score << '\n';
}

void Game::LoadHighScores()
{
	std::ifstream file("scores.txt");

	if (!file.is_open())
		return;

	m_highScores.clear();
	std::string name;
	int score = 0;

	while (file >> name >> score)
		m_highScores.push_back({ name, score });
}
void Game::RenderHighScores(std::ostringstream& frame) const
{
	frame << "\nHigh Scores\n";

	if (m_highScores.empty())
	{
		frame << "  No scores yet\n";
		return;
	}

	for (int i = 0; i < static_cast<int>(m_highScores.size()); ++i)
	{
		frame << "  " << (i + 1) << ". "
			<< m_highScores[i].name << " - "
			<< m_highScores[i].score << '\n';
	}
}

int Game::CalculateScore(int clearedLines) const
{
	switch (clearedLines)
	{
	case 1:
		return 100 * m_level;
	case 2:
		return 300 * m_level;
	case 3:
		return 500 * m_level;
	case 4:
		return 800 * m_level;
	default:
		return 0;
	}
}

int Game::CalculateTSpinScore(int clearedLines) const
{
	switch (clearedLines)
	{
	case 0:
		return TSpinNoLineScore * m_level;
	case 1:
		return TSpinSingleScore * m_level;
	case 2:
		return TSpinDoubleScore * m_level;
	case 3:
		return TSpinTripleScore * m_level;
	default:
		return 0;
	}
}

void Game::UpdateLevel()
{
	m_level = (m_totalLines / 10) + 1;

	const int fallInterval = InitialFallIntervalMs - (m_level - 1) * FallIntervalDecreasePerLevel;

	m_fallInterval = std::chrono::milliseconds(std::max(fallInterval, MinFallIntervalMs));
}

void Game::SpawnNextPiece()
{
	m_currentPiece = m_nextPiece;
	m_currentPiece.SetPosition(Board::Width / 2 - 1, 0);
	m_lastMoveWasRotation = false;

	m_nextPiece = Tetromino(CreateRandomTetrominoType());
}

void Game::ProcessLockAndResolve()
{
	const bool isTSpin = DetectTSpin();

	m_board.LockPiece(m_currentPiece);

	const int clearedLines = m_board.ClearLines();

	if (isTSpin && clearedLines == 0)
	{
		m_score += CalculateTSpinScore(clearedLines);
		m_lastClearMessage = "T-Spin";
	}
	else if (clearedLines == 0)
	{
		m_lastClearMessage.clear();
	}

	if (clearedLines > 0)
	{
		const int lineClearScore = isTSpin ? CalculateTSpinScore(clearedLines) : CalculateScore(clearedLines);
		const bool isDifficultClear = (clearedLines == 4) || isTSpin;
		const char* clearNames[] = { "", "Single", "Double", "Triple", "Tetris" };

		if (isTSpin)
			m_lastClearMessage = std::string("T-Spin ") + clearNames[clearedLines];
		else
			m_lastClearMessage = clearNames[clearedLines];

		++m_combo;
		m_totalLines += clearedLines;
		m_score += lineClearScore;
		m_score += m_combo * ComboScorePerStep * m_level;

		if (isDifficultClear)
		{
			if (m_isBackToBackActive)
				m_score += lineClearScore / 2;

			m_isBackToBackActive = true;
		}
		else
		{
			m_isBackToBackActive = false;
		}

		UpdateLevel();
	}
	else
	{
		m_combo = -1;
	}

	m_isLockRequired = false;
	ResetLockDelay();
	SpawnNextPiece();
	m_canHold = true;
	m_lastFallTime = std::chrono::steady_clock::now();

	if (!m_board.CanPlace(m_currentPiece))
	{
		TransitionTo(GameState::GameOver);
		m_needsHighScoreName = true;
	}
}

bool Game::TryMoveCurrentPiece(int dx, int dy, bool lockOnFail)
{
	m_currentPiece.Move(dx, dy);

	if (m_board.CanPlace(m_currentPiece))
	{
		RefreshLockDelayAfterSuccessfulMove();
		return true;
	}

	m_currentPiece.Move(-dx, -dy);

	if (lockOnFail)
		StartLockDelay();

	return false;
}

bool Game::TryRotateCurrentPiece(RotationDirection direction)
{
	const int oldRotationIndex = m_currentPiece.GetRotation() / 90;
	const auto kicks = GetSrsKicks(m_currentPiece.GetType(), oldRotationIndex, direction);

	if (direction == RotationDirection::Clockwise)
		m_currentPiece.RotateCW();
	else
		m_currentPiece.RotateCCW();

	for (const Point& kick : kicks)
	{
		m_currentPiece.Move(kick.x, kick.y);

		if (m_board.CanPlace(m_currentPiece))
		{
			RefreshLockDelayAfterSuccessfulMove();
			m_lastMoveWasRotation = true;
			return true;
		}

		m_currentPiece.Move(-kick.x, -kick.y);
	}

	if (direction == RotationDirection::Clockwise)
		m_currentPiece.RotateCCW();
	else
		m_currentPiece.RotateCW();

	return false;
}

void Game::HardDropCurrentPiece()
{
	int dropped = 0;

	while (TryMoveCurrentPiece(0, 1, false))
		++dropped;

	m_score += dropped * 2;

	m_isLockRequired = true;
}

void Game::HoldCurrentPiece()
{
	if (!m_canHold)
		return;

	const TetrominoType currentType = m_currentPiece.GetType();

	if (m_hasHoldPiece)
	{
		const TetrominoType heldType = m_holdPiece.GetType();
		m_holdPiece = Tetromino(currentType);
		m_currentPiece = Tetromino(heldType);
		m_currentPiece.SetPosition(Board::Width / 2 - 1, 0);
	}
	else
	{
		m_holdPiece = Tetromino(currentType);
		m_hasHoldPiece = true;
		SpawnNextPiece();
	}

	m_canHold = false;
	m_isLockRequired = false;
	m_lastMoveWasRotation = false;
	ResetLockDelay();
	m_lastFallTime = std::chrono::steady_clock::now();

	if (!m_board.CanPlace(m_currentPiece))
	{
		TransitionTo(GameState::GameOver);
		m_needsHighScoreName = true;
	}
}

TetrominoType Game::CreateRandomTetrominoType()
{
	if (m_pieceBag.empty())
		RefillPieceBag();

	const TetrominoType type = m_pieceBag.back();
	m_pieceBag.pop_back();
	return type;
}

void Game::RefillPieceBag()
{
	m_pieceBag = {
		TetrominoType::I,
		TetrominoType::J,
		TetrominoType::L,
		TetrominoType::O,
		TetrominoType::S,
		TetrominoType::T,
		TetrominoType::Z
	};

	std::shuffle(m_pieceBag.begin(), m_pieceBag.end(), m_randomEngine);
}

void Game::StartLockDelay()
{
	if (m_isTouchingGround)
		return;

	m_isTouchingGround = true;
	m_lockResetCount = 0;
	m_lockStartTime = std::chrono::steady_clock::now();
}

void Game::ResetLockDelay()
{
	m_isTouchingGround = false;
	m_lockResetCount = 0;
	m_lockStartTime = std::chrono::steady_clock::now();
}

void Game::RefreshLockDelayAfterSuccessfulMove()
{
	if (!IsCurrentPieceTouchingGround())
	{
		ResetLockDelay();
		return;
	}

	if (!m_isTouchingGround)
	{
		StartLockDelay();
		return;
	}

	if (m_lockResetCount >= MaxLockResetCount)
		return;

	++m_lockResetCount;
	m_lockStartTime = std::chrono::steady_clock::now();
}

bool Game::IsCurrentPieceTouchingGround() const
{
	Tetromino testPiece = m_currentPiece;
	testPiece.Move(0, 1);

	return !m_board.CanPlace(testPiece);
}

bool Game::DetectTSpin() const
{
	if (m_currentPiece.GetType() != TetrominoType::T)
		return false;

	if (!m_lastMoveWasRotation)
		return false;

	const Point position = m_currentPiece.GetPosition();
	const Point center{ position.x + 1, position.y + 1 };
	const std::array<Point, 4> corners{
		Point{ center.x - 1, center.y - 1 },
		Point{ center.x + 1, center.y - 1 },
		Point{ center.x - 1, center.y + 1 },
		Point{ center.x + 1, center.y + 1 }
	};

	int blockedCorners = 0;

	for (const Point& corner : corners)
	{
		if (!m_board.IsInside(corner) || m_board.IsCellFilled(corner))
			++blockedCorners;
	}

	return blockedCorners >= 3;
}

Tetromino Game::GetGhostPiece() const
{
	Tetromino ghostPiece = m_currentPiece;

	while (true)
	{
		ghostPiece.Move(0, 1);

		if (!m_board.CanPlace(ghostPiece))
		{
			ghostPiece.Move(0, -1);
			break;
		}
	}

	return ghostPiece;
}
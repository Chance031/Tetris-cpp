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
// 크로스플랫폼 빌드 시 별도 입력 계층으로 대체해야 한다.
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

// 저장된 하이스코어를 불러오고 게임 상태를 Title로 초기화한다.
void Game::Initialize()
{
	LoadHighScores();
	TransitionTo(GameState::Title);
}

// 상태에 따라 입력, 업데이트, 렌더링 루프를 전환하면서 게임 전체를 실행한다.
void Game::Run()
{
	// 각 프레임 사이에 50ms 지연을 두어 루프가 너무 빠르게 돌지 않도록 한다.
	constexpr auto FrameDelay = std::chrono::milliseconds(50);

	// 화면을 지우고 커서를 홈 위치로 옮긴 뒤, 깜빡이는 커서를 숨긴다.
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
	// 숨겨둔 커서를 다시 보이게 한다.
	std::cout << "\x1B[?25h" << std::flush;
}

// 새 게임을 시작하기 위해 필요한 모든 진행 상태를 초기값으로 되돌린다.
// 첫 피스를 준비한 뒤 게임 상태를 Playing으로 전환한다.
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

// 게임 상태를 새로운 상태로 전환한다.
void Game::TransitionTo(GameState newState)
{
	m_state = newState;
}

// 플레이 중 입력을 처리한다.
// 종료, 일시 정지, 이동, 회전, 드롭, 홀드 입력을 담당한다.
void Game::HandleInput()
{
	// 현재 눌린 키가 없으면 아무 것도 하지 않고 바로 끝낸다.
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

	// 방향키는 확장 키 prefix를 먼저 읽은 뒤 실제 키 코드를 한 번 더 읽는다.
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

// 타이틀 상태에서 시작 또는 종료 입력을 처리한다.
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

// 일시 정지 상태에서 재개 또는 종료 입력을 처리한다.
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

// 게임오버 상태에서 하이스코어 입력과 재시작 또는 종료 입력을 처리한다.
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

// 현재 프레임의 게임 상태를 갱신한다.
// 락 지연 처리와 자동 낙하 타이머를 포함한다.
void Game::Update()
{
	if (m_isLockRequired)
	{
		ProcessLockAndResolve();
		return;
	}

	const auto now = std::chrono::steady_clock::now();

	// 바닥에 닿은 뒤 락 딜레이가 끝나면 현재 피스를 확정한다.
	if (m_isTouchingGround && now - m_lockStartTime >= m_lockDelay)
	{
		m_isLockRequired = true;
		ProcessLockAndResolve();
		return;
	}

	// 아직 자동 낙하 시점이 아니면 이번 프레임에는 아무 것도 하지 않는다.
	if (now - m_lastFallTime < m_fallInterval)
		return;

	m_lastFallTime = now;
	TryMoveCurrentPiece(0, 1, true);
}

// 현재 상태에 맞는 화면을 렌더링한다.
// 타이틀, 종료, 플레이 중 화면을 각각 다르게 구성한다.
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

// 게임이 끝난 뒤 하이스코어 이름을 입력받아 점수를 하이스코어 목록에 추가한다.
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

		// 공백 이하 제어 문자와 출력 불가능한 문자는 무시한다.
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

// 새 점수를 하이스코어 목록에 추가하고, 정렬 및 개수 제한을 적용한 뒤 저장한다.
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

// 현재 하이스코어 목록을 scores.txt 파일에 저장한다.
void Game::SaveHighScores() const
{
	std::ofstream file("scores.txt");

	for (const HighScoreEntry& entry : m_highScores)
		file << entry.name << ' ' << entry.score << '\n';
}

// 저장된 scores.txt 파일을 읽어 m_highScores 벡터를 다시 채운다.
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

// 하이스코어 목록을 화면 출력용 문자열 버퍼에 렌더링한다.
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

// 지운 줄 수와 현재 레벨에 따라 일반 라인 클리어 점수를 계산한다.
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

// T-Spin으로 지운 줄 수와 현재 레벨에 따라 점수를 계산한다.
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

// 누적 삭제 줄 수에 따라 레벨과 낙하 속도를 갱신한다.
void Game::UpdateLevel()
{
	m_level = (m_totalLines / 10) + 1;

	const int fallInterval = InitialFallIntervalMs - (m_level - 1) * FallIntervalDecreasePerLevel;

	m_fallInterval = std::chrono::milliseconds(std::max(fallInterval, MinFallIntervalMs));
}

// 현재 피스를 기존 next 피스로 교체해 시작 위치에 배치하고, 새로운 next 피스를 생성한다.
void Game::SpawnNextPiece()
{
	m_currentPiece = m_nextPiece;
	m_currentPiece.SetPosition(Board::Width / 2 - 1, 0);
	m_lastMoveWasRotation = false;

	m_nextPiece = Tetromino(CreateRandomTetrominoType());
}

// 현재 피스를 보드에 고정한 뒤, 라인 삭제와 점수 정산, 다음 피스 스폰까지 처리한다.
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

	// 새 피스를 놓을 수 없으면 게임오버로 전환한다.
	if (!m_board.CanPlace(m_currentPiece))
	{
		TransitionTo(GameState::GameOver);
		m_needsHighScoreName = true;
	}
}

// 현재 피스 이동을 시도하고, 실패하면 원래 위치로 되돌린다.
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

// SRS 벽킥 규칙을 적용해 회전을 시도하고, 실패하면 회전 상태를 원래대로 되돌린다.
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

// 현재 피스를 가능한 한 아래로 즉시 떨어뜨리고, 이동한 칸 수만큼 점수를 준 뒤 바로 고정 단계로 넘긴다.
void Game::HardDropCurrentPiece()
{
	int dropped = 0;

	while (TryMoveCurrentPiece(0, 1, false))
		++dropped;

	m_score += dropped * 2;

	m_isLockRequired = true;
}

// 현재 피스를 홀드 칸에 저장하거나 기존 홀드 피스와 교체한 뒤, 관련 상태를 초기화한다.
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

// 7-bag에서 다음 블록 타입을 하나 꺼낸다.
TetrominoType Game::CreateRandomTetrominoType()
{
	if (m_pieceBag.empty())
		RefillPieceBag();

	const TetrominoType type = m_pieceBag.back();
	m_pieceBag.pop_back();
	return type;
}

// I, J, L, O, S, T, Z를 각각 한 번씩 bag에 넣고 섞는다.
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

// 현재 블록이 바닥에 처음 닿았을 때 락 딜레이를 시작한다.
void Game::StartLockDelay()
{
	if (m_isTouchingGround)
		return;

	m_isTouchingGround = true;
	m_lockResetCount = 0;
	m_lockStartTime = std::chrono::steady_clock::now();
}

// 락 딜레이 상태를 초기화한다.
void Game::ResetLockDelay()
{
	m_isTouchingGround = false;
	m_lockResetCount = 0;
	m_lockStartTime = std::chrono::steady_clock::now();
}

// 이동이나 회전에 성공한 뒤, 현재 바닥 접촉 상태에 따라 락 딜레이를 갱신한다.
void Game::RefreshLockDelayAfterSuccessfulMove()
{
	// 바닥에 닿아 있지 않으면 락 딜레이를 초기화한다.
	if (!IsCurrentPieceTouchingGround())
	{
		ResetLockDelay();
		return;
	}

	// 바닥에 처음 닿은 상태면 락 딜레이를 시작한다.
	if (!m_isTouchingGround)
	{
		StartLockDelay();
		return;
	}

	// 이미 바닥에 닿아 있으면, 허용 횟수 안에서 락 타이머를 다시 시작한다.
	if (m_lockResetCount >= MaxLockResetCount)
		return;

	++m_lockResetCount;
	m_lockStartTime = std::chrono::steady_clock::now();
}

// 현재 피스를 한 칸 더 내릴 수 없는 상태인지 검사한다.
bool Game::IsCurrentPieceTouchingGround() const
{
	Tetromino testPiece = m_currentPiece;
	testPiece.Move(0, 1);

	return !m_board.CanPlace(testPiece);
}

// 현재 상태가 T-Spin 조건을 만족하는지 판정한다.
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

	// T 중심 주변 4개 코너 중 3개 이상이 막혀 있는지 확인한다.
	for (const Point& corner : corners)
	{
		if (!m_board.IsInside(corner) || m_board.IsCellFilled(corner))
			++blockedCorners;
	}

	return blockedCorners >= 3;
}

// 현재 피스가 즉시 떨어졌을 때 도달할 위치를 고스트 피스로 계산해 반환한다.
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
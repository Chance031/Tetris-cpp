#include "Game.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <conio.h>

namespace
{
	constexpr int KeyEscape = 27;
	constexpr int KeyEnter = 13;
	constexpr int KeyExtendedPrefix = 224;
	constexpr int KeyExtendedPrefixAlt = 0;
	constexpr int KeyArrowLeft = 75;
	constexpr int KeyArrowRight = 77;
	constexpr int KeyArrowDown = 80;
	constexpr int NextPiecePreviewSize = 4;

	bool ContainsPoint(const std::array<Point, 4>& blocks, Point point)
	{
		for (const Point& block : blocks)
		{
			if (block.x == point.x && block.y == point.y)
				return true;
		}

		return false;
	}

	char GetBoardCellDisplay(const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, Point point)
	{
		if (ContainsPoint(currentBlocks, point))
			return '@';

		if (board.IsCellFilled(point))
			return '#';

		if (ContainsPoint(ghostBlocks, point))
			return '+';

		return '.';
	}

	void RenderPiecePreviewRow(std::ostringstream& frame, const std::array<Point, 4>& blocks, int previewY)
	{
		frame << "   ";

		for (int previewX = 0; previewX < NextPiecePreviewSize; ++previewX)
		{
			Point previewPoint{ previewX, previewY };
			frame << (ContainsPoint(blocks, previewPoint) ? '@' : '.');
		}
	}
}

Game::Game() : m_randomEngine(std::random_device{}())
{
}

void Game::Initialize()
{
	m_state = GameState::Title;
}

void Game::Run()
{
	constexpr auto FrameDelay = std::chrono::milliseconds(50);

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
			m_state = GameState::Exit;
	}

	Render();
	std::cout << "\x1B[?25h" << std::flush;
}

void Game::StartNewSession()
{
	m_board.Reset();
	m_score = 0;
	m_level = 1;
	m_totalLines = 0;
	m_isLockRequired = false;
	ResetLockDelay();
	m_hasHoldPiece = false;
	m_canHold = true;
	m_state = GameState::Playing;

	m_fallInterval = std::chrono::milliseconds(InitialFallIntervalMs);
	m_lastFallTime = std::chrono::steady_clock::now();

	m_nextPiece = Tetromino(CreateRandomTetrominoType());
	SpawnNextPiece();
}

void Game::HandleInput()
{
	if (!_kbhit())
		return;

	int key = _getch();

	if (key == KeyEscape || key == 'q' || key == 'Q')
	{
		m_state = GameState::Exit;
		return;
	}

	if (key == 'p' || key == 'P')
	{
		m_state = GameState::Paused;
		return;
	}

	if (key == KeyExtendedPrefix || key == KeyExtendedPrefixAlt)
	{
		key = _getch();

		switch (key)
		{
		case KeyArrowLeft:
			TryMoveCurrentPiece(-1, 0, false);
			break;
		case KeyArrowRight:
			TryMoveCurrentPiece(1, 0, false);
			break;
		case KeyArrowDown:
			TryMoveCurrentPiece(0, 1, true);
			break;
		}

		return;
	}

	if (key == 'z' || key == 'Z')
	{
		TryRotateCurrentPieceCW();
	}
	else if (key == ' ')
	{
		HardDropCurrentPiece();
	}
	else if (key == 'c' || key == 'C')
	{
		HoldCurrentPiece();
	}
#ifdef _DEBUG
	else if (key == 'b' || key == 'B')
	{
		m_board.FillDebugLine(Board::Height - 1, Board::Width / 2);
	}
#endif
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
			m_state = GameState::Exit;
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
			m_state = GameState::Playing;
			m_lastFallTime = std::chrono::steady_clock::now();

			if (m_isTouchingGround)
				m_lockStartTime = std::chrono::steady_clock::now();
		}
		else if (key == KeyEscape || key == 'q' || key == 'Q')
		{
			m_state = GameState::Exit;
		}
	}
}
void Game::HandleGameOverInput()
{
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
			m_state = GameState::Exit;
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
		<< "  Lines: " << m_totalLines << '\n';

	for (int y = 0; y < Board::Height; ++y)
	{
		for (int x = 0; x < Board::Width; ++x)
		{
			Point currentPoint{ x, y };
			frame << GetBoardCellDisplay(m_board, currentBlocks, ghostBlocks, currentPoint);
		}

		if (y == 0)
		{
			frame << "   Next Piece";
		}
		else if (y >= 1 && y <= 4)
		{
			RenderPiecePreviewRow(frame, nextBlocks, y - 1);
		}
		else if (y == 6)
		{
			frame << "   Hold Piece";
		}
		else if (y >= 7 && y <= 10)
		{
			if (m_hasHoldPiece)
				RenderPiecePreviewRow(frame, holdBlocks, y - 7);
			else
				frame << "   ....";
		}

		frame << '\n';
	}

	frame << "\nControls: Left/Right = Move, Down = Soft Drop, Z = Rotate, Space = Hard Drop, C = Hold, P = Pause, Q/Esc = Quit\n";
#ifdef _DEBUG
	frame << "Debug: B = Fill Bottom Line\n";
#endif

	if (m_state == GameState::Paused)
		frame << "\nPaused - Press P to resume, Q/Esc to quit\n";
	else if (m_state == GameState::GameOver)
		frame << "\nGame Over - Press R to restart, Q/Esc to quit\n";

	std::cout << frame.str() << std::flush;
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

	m_nextPiece = Tetromino(CreateRandomTetrominoType());
}

void Game::ProcessLockAndResolve()
{
	m_board.LockPiece(m_currentPiece);

	const int clearedLines = m_board.ClearLines();

	if (clearedLines > 0)
	{
		m_totalLines += clearedLines;
		m_score += CalculateScore(clearedLines);
		UpdateLevel();
	}

	m_isLockRequired = false;
	ResetLockDelay();
	SpawnNextPiece();
	m_canHold = true;
	m_lastFallTime = std::chrono::steady_clock::now();

	if (!m_board.CanPlace(m_currentPiece))
		m_state = GameState::GameOver;
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

bool Game::TryRotateCurrentPieceCW()
{
	m_currentPiece.RotateCW();

	if (m_board.CanPlace(m_currentPiece))
	{
		RefreshLockDelayAfterSuccessfulMove();
		return true;
	}

	for (int kick : { 1, -1, 2, -2 })
	{
		m_currentPiece.Move(kick, 0);

		if (m_board.CanPlace(m_currentPiece))
		{
			RefreshLockDelayAfterSuccessfulMove();
			return true;
		}

		m_currentPiece.Move(-kick, 0);
	}

	m_currentPiece.RotateCCW();
	return false;
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
	ResetLockDelay();
	m_lastFallTime = std::chrono::steady_clock::now();

	if (!m_board.CanPlace(m_currentPiece))
		m_state = GameState::GameOver;
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


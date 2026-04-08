#include "Game.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <conio.h>

namespace
{
	constexpr int KeyEscape = 27;
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
	StartNewSession();
}

void Game::Run()
{
	constexpr auto FrameDelay = std::chrono::milliseconds(50);

	std::cout << "\x1B[2J\x1B[H\x1B[?25l";

	while (m_state != GameState::Exit)
	{
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

		if (m_state == GameState::GameOver)
			HandleGameOverInput();
		else if (m_state != GameState::Exit)
			m_state = GameState::Exit;
	}

	std::cout << "\x1B[?25h" << std::flush;
}

void Game::StartNewSession()
{
	m_board.Reset();
	m_score = 0;
	m_level = 1;
	m_totalLines = 0;
	m_isLockRequired = false;
	m_state = GameState::Playing;

	m_fallInterval = std::chrono::milliseconds(800);
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

	if (now - m_lastFallTime < m_fallInterval)
		return;

	m_lastFallTime = now;

	TryMoveCurrentPiece(0, 1, true);

	if (m_isLockRequired)
		ProcessLockAndResolve();
}

void Game::Render()
{
	const auto currentBlocks = m_currentPiece.GetBlockLocations();
	const auto nextBlocks = m_nextPiece.GetBlockLocations();
	std::ostringstream frame;

	frame << "\x1B[H";
	frame << "Score: " << m_score
		<< "  Level: " << m_level
		<< "  Lines: " << m_totalLines << '\n';

	for (int y = 0; y < Board::Height; ++y)
	{
		for (int x = 0; x < Board::Width; ++x)
		{
			Point currentPoint{ x, y };
			bool isCurrentPieceCell = false;

			for (const Point& block : currentBlocks)
			{
				if (block.x == currentPoint.x && block.y == currentPoint.y)
				{
					isCurrentPieceCell = true;
					break;
				}
			}

			if (isCurrentPieceCell)
				frame << '@';
			else if (m_board.IsCellFilled(currentPoint))
				frame << '#';
			else
				frame << '.';
		}

		if (y == 0)
		{
			frame << "   Next Piece";
		}
		else if (y >= 1 && y <= 4)
		{
			frame << "   ";
			const int previewY = y - 1;

			for (int previewX = 0; previewX < 4; ++previewX)
			{
				bool isNextPieceCell = false;

				for (const Point& block : nextBlocks)
				{
					if (block.x == previewX && block.y == previewY)
					{
						isNextPieceCell = true;
						break;
					}
				}

				frame << (isNextPieceCell ? '@' : '.');
			}
		}

		frame << '\n';
	}

	frame << "\nControls: Left/Right = Move, Down = Soft Drop, Z = Rotate, Space = Hard Drop, Q/Esc = Quit\n";

	if (m_state == GameState::GameOver)
		frame << "\nGame Over - Press R to restart, Q/Esc to quit\n";
	else if (m_state == GameState::Exit)
		frame << "\nQuit\n";

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

	const int fallInterval = 800 - (m_level - 1) * 50;

	if (fallInterval < 100)
		m_fallInterval = std::chrono::milliseconds(100);
	else
		m_fallInterval = std::chrono::milliseconds(fallInterval);
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
	SpawnNextPiece();
	m_lastFallTime = std::chrono::steady_clock::now();

	if (!m_board.CanPlace(m_currentPiece))
		m_state = GameState::GameOver;
}

bool Game::TryMoveCurrentPiece(int dx, int dy, bool lockOnFail)
{
	m_currentPiece.Move(dx, dy);

	if (m_board.CanPlace(m_currentPiece))
		return true;

	m_currentPiece.Move(-dx, -dy);

	if (lockOnFail)
		m_isLockRequired = true;

	return false;
}

bool Game::TryRotateCurrentPieceCW()
{
	m_currentPiece.RotateCW();

	if (m_board.CanPlace(m_currentPiece))
		return true;

	m_currentPiece.RotateCCW();
	return false;
}

void Game::HardDropCurrentPiece()
{
	while (TryMoveCurrentPiece(0, 1, false))
	{
	}

	m_isLockRequired = true;
}

TetrominoType Game::CreateRandomTetrominoType()
{
	return static_cast<TetrominoType>(m_pieceDistribution(m_randomEngine));
}


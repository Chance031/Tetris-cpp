#include "Game.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>
#include <conio.h>

Game::Game() : m_randomEngine(std::random_device{}())
{
}

void Game::Initialize()
{
	StartNewSession();
}

void Game::Run()
{
	constexpr int TestFrameCount = 400;
	constexpr auto FrameDelay = std::chrono::milliseconds(150);

	std::cout << "\x1B[2J\x1B[H\x1B[?25l";

	for (int frame = 0; frame < TestFrameCount && m_state == GameState::Playing; ++frame)
	{
		HandleInput();
		Update();
		Render();

		std::this_thread::sleep_for(FrameDelay);
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

	m_nextPiece = Tetromino(CreateRandomTetrominoType());
	SpawnNextPiece();
}

void Game::HandleInput()
{
	if (!_kbhit())
		return;

	const int key = _getch();

	if (key == 'a' || key == 'A')
	{
		TryMoveCurrentPiece(-1, 0, false);
	}
	else if (key == 'd' || key == 'D')
	{
		TryMoveCurrentPiece(1, 0, false);
	}
	else if (key == 'z' || key == 'Z')
	{
		TryRotateCurrentPieceCW();
	}
	else if (key == 's' || key == 'S')
	{
		TryMoveCurrentPiece(0, 1, true);
	}
}

void Game::Update()
{
	TryMoveCurrentPiece(0, 1, true);

	if (m_isLockRequired)
		ProcessLockAndResolve();
}

void Game::Render()
{
	const auto currentBlocks = m_currentPiece.GetBlockLocations();
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

		frame << '\n';
	}

	std::cout << frame.str() << std::flush;
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
		// TODO: 점수 계산
	}

	m_isLockRequired = false;
	SpawnNextPiece();

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

TetrominoType Game::CreateRandomTetrominoType()
{
	return static_cast<TetrominoType>(m_pieceDistribution(m_randomEngine));
}

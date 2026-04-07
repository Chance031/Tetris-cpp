#include "Game.h"

#include <iostream>

Game::Game() : m_randomEngine(std::random_device{}())
{
}

void Game::Initialize()
{
	StartNewSession();
}

void Game::Run()
{
	while (m_state == GameState::Playing)
	{
		HandleInput();
		Update();
		Render();

		break; // TODO: 실제 루프가 준비되면 제거
	}
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
}

void Game::Update()
{
}

void Game::Render()
{
	const auto currentBlocks = m_currentPiece.GetBlockLocations();

	std::cout << "Score: " << m_score
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
				std::cout << '@';
			else if (m_board.IsCellFilled(currentPoint))
				std::cout << '#';
			else
				std::cout << '.';
		}

		std::cout << '\n';
	}
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

TetrominoType Game::CreateRandomTetrominoType()
{
	return static_cast<TetrominoType>(m_pieceDistribution(m_randomEngine));
}

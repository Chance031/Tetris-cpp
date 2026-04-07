#include "Game.h"

Game::Game() : m_randomEngine(std::random_device{}())
{
}

void Game::Initialize()
{
	StartNewSession();
}

void Game::Run()
{
	// TODO: 입력, 업데이트, 렌더링 루프 연결
}

void Game::StartNewSession()
{
	m_board.Reset();
	m_score = 0;
	m_level = 1;
	m_totalLines = 0;
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
	return static_cast<TetrominoType> (m_pieceDistribution(m_randomEngine));
}

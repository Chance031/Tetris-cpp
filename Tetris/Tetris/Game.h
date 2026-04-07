#pragma once

#include "Board.h"
#include "Tetromino.h"
#include "Types.h"

#include <random>

// Game 루프 전체를 조율하는 최상위 클래스다.
// 입력 처리, 상태 업데이트, 렌더링 순서를 관리하고
// Board와 Tetromino의 상호작용을 중재한다.
class Game
{
public:
	Game();

	void Initialize();
	void Run();

private:
	void StartNewSession();

	void HandleInput();
	void Update();
	void Render();

	void SpawnNextPiece();
	void ProcessLockAndResolve();

	bool TryMoveCurrentPiece(int dx, int dy, bool lockOnFail);
	bool TryRotateCurrentPieceCW();
	TetrominoType CreateRandomTetrominoType();

private:
	Board m_board;
	Tetromino m_currentPiece;
	Tetromino m_nextPiece;

	GameState m_state = GameState::Title;
	bool m_isLockRequired = false; // true이면 다음 Update()에서 현재 블록을 보드에 고정한다.

	int m_score = 0;
	int m_level = 1;
	int m_totalLines = 0;

	std::mt19937 m_randomEngine;
	std::uniform_int_distribution<int> m_pieceDistribution{ 0, 6 };
};

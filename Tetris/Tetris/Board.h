#pragma once

#include "Tetromino.h"

// 테트리스 보드의 고정된 블록 상태를 관리한다.
//
// Board는 현재 조작 중인 블록을 직접 움직이지 않는다.
// Tetromino가 차지하는 좌표를 기준으로 배치 가능 여부, 고정, 라인 삭제를 처리한다.
class Board
{
public:
	Board() = default;

	static constexpr int Width = 10;
	static constexpr int Height = 20;

	void Reset();

	bool CanPlace(const Tetromino& tetromino) const;
	void LockPiece(const Tetromino& tetromino);

	int ClearLines();

	bool IsInside(Point point) const;
	bool IsCellEmpty(Point point) const;
	bool IsCellFilled(Point point) const;

#ifdef _DEBUG
	void FillDebugLine(int y, int emptyX);
	void SetDebugCell(Point point, bool isFilled);
#endif

private:
	// 0은 빈 칸, 1은 고정된 블록이 있는 칸을 뜻한다.
	// 인덱스 순서는 m_cells[y][x]다.
	int m_cells[Height][Width] = {};
};

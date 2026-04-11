#include "Board.h"

#include <cstring>

// 보드의 모든 칸을 빈 상태로 초기화한다.
void Board::Reset()
{
	std::memset(m_cells, 0, sizeof(m_cells));
}

// 테트로미노가 보드 안에 겹치지 않고 배치 가능한지 확인한다.
bool Board::CanPlace(const Tetromino& tetromino) const
{
	const auto blocks = tetromino.GetBlockLocations();

	for (const Point& block : blocks)
	{
		// 보드 범위를 벗어나면 배치할 수 없다.
		if (!IsInside(block))
			return false;

		// 이미 고정된 블록이 있는 칸에도 배치할 수 없다.
		if (!IsCellEmpty(block))
			return false;
	}

	return true;
}

// 테트로미노가 차지하는 좌표를 보드에 고정한다.
void Board::LockPiece(const Tetromino& tetromino)
{
	const auto blocks = tetromino.GetBlockLocations();

	for (const Point& block : blocks)
	{
		// 보드 범위 안에 있는 칸만 고정한다.
		if (IsInside(block))
			m_cells[block.y][block.x] = 1;
	}
}

// 아래 줄부터 검사하며 가득 찬 줄을 삭제하고, 삭제한 줄 수를 반환한다.
int Board::ClearLines()
{
	int clearedLines = 0;
	int y = Height - 1;

	// 아래 줄부터 검사하면서 가득 찬 줄을 삭제한다.
	while (y >= 0)
	{
		bool isFullLine = true;

		for (int x = 0; x < Width; ++x)
		{
			if (m_cells[y][x] == 0)
			{
				isFullLine = false;
				break;
			}
		}

		if (!isFullLine)
		{
			--y;
			continue;
		}

		++clearedLines;

		// 현재 줄 위의 모든 줄을 한 칸씩 아래로 내린다.
		for (int row = y; row > 0; --row)
		{
			for (int x = 0; x < Width; ++x)
				m_cells[row][x] = m_cells[row - 1][x];
		}

		// 최상단 줄은 빈 칸으로 초기화한다.
		for (int x = 0; x < Width; ++x)
			m_cells[0][x] = 0;

		// y는 감소하지 않는다. 위 줄이 내려왔으므로 같은 y를 다시 검사한다.
	}

	return clearedLines;
}

// 좌표가 보드 범위 안에 있는지 확인한다.
bool Board::IsInside(Point point) const
{
	return point.x >= 0 && point.x < Width && point.y >= 0 && point.y < Height;
}

// 좌표가 보드 안에 있고, 비어 있는 칸인지 확인한다.
bool Board::IsCellEmpty(Point point) const
{
	return IsInside(point) && m_cells[point.y][point.x] == 0;
}

// 좌표가 보드 안에 있고, 고정된 블록이 있는 칸인지 확인한다.
bool Board::IsCellFilled(Point point) const
{
	return IsInside(point) && m_cells[point.y][point.x] != 0;
}
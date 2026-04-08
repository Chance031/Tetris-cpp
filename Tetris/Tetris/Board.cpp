#include "Board.h"

#include <cstring>

void Board::Reset()
{
	std::memset(m_cells, 0, sizeof(m_cells));
}

bool Board::CanPlace(const Tetromino& tetromino) const
{
	const auto blocks = tetromino.GetBlockLocations();

	for (const Point& block : blocks)
	{
		if (!IsInside(block))
			return false;

		if (!IsCellEmpty(block))
			return false;
	}

	return true;
}

void Board::LockPiece(const Tetromino& tetromino)
{
	const auto blocks = tetromino.GetBlockLocations();

	for (const Point& block : blocks)
	{
		if (IsInside(block))
			m_cells[block.y][block.x] = 1;
	}
}

int Board::ClearLines()
{
	int clearedLines = 0;
	int y = Height - 1;

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

		for (int row = y; row > 0; --row)
		{
			for (int x = 0; x < Width; ++x)
				m_cells[row][x] = m_cells[row - 1][x];
		}

		for (int x = 0; x < Width; ++x)
			m_cells[0][x] = 0;

		// y는 감소하지 않는다. 위 줄이 내려왔으므로 같은 y를 다시 검사한다.
	}

	return clearedLines;
}

bool Board::IsInside(Point point) const
{
	return point.x >= 0 && point.x < Width && point.y >= 0 && point.y < Height;
}

bool Board::IsCellEmpty(Point point) const
{
	return IsInside(point) && m_cells[point.y][point.x] == 0;
}

bool Board::IsCellFilled(Point point) const
{
	return IsInside(point) && m_cells[point.y][point.x] != 0;
}
#include "Board.h"

void Board::Reset()
{
	for (int y = 0; y < Height; ++y)
	{
		for (int x = 0; x < Width; ++x)
			m_cells[y][x] = 0;
	}
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

	for (int y = Height - 1; y >= 0; --y)
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
			continue;

		++clearedLines;

		// 삭제된 줄 위의 모든 줄을 한 칸씩 아래로 내린다.
		for (int row = y; row > 0; --row)
		{
			for (int x = 0; x < Width; ++x)
				m_cells[row][x] = m_cells[row - 1][x];
		}

		for (int x = 0; x < Width; ++x)
			m_cells[0][x] = 0;

		// 위 줄이 내려왔으므로 같은 y 위치를 다시 검사한다.
		++y;
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

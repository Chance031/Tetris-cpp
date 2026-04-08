#include "Renderer.h"

namespace
{
	constexpr int NextPiecePreviewSize = 4;
}

namespace Renderer
{
	bool ContainsPoint(const std::array<Point, 4>& blocks, Point point)
	{
		for (const Point& block : blocks)
		{
			if (block.x == point.x && block.y == point.y)
				return true;
		}

		return false;
	}

	void AppendColoredCell(std::ostringstream& frame, char cell)
	{
		switch (cell)
		{
		case '@':
			frame << "\x1B[36m@\x1B[0m";
			break;
		case '#':
			frame << "\x1B[37m#\x1B[0m";
			break;
		case '+':
			frame << "\x1B[90m+\x1B[0m";
			break;
		default:
			frame << cell;
			break;
		}
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
			AppendColoredCell(frame, ContainsPoint(blocks, previewPoint) ? '@' : '.');
		}
	}

	void RenderBoardRow(std::ostringstream& frame, const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, int y)
	{
		for (int x = 0; x < Board::Width; ++x)
		{
			Point currentPoint{ x, y };
			AppendColoredCell(frame, GetBoardCellDisplay(board, currentBlocks, ghostBlocks, currentPoint));
		}
	}

	void RenderSidePanelRow(std::ostringstream& frame, const std::array<Point, 4>& nextBlocks, const std::array<Point, 4>& holdBlocks, bool hasHoldPiece, int y)
	{
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
			if (hasHoldPiece)
				RenderPiecePreviewRow(frame, holdBlocks, y - 7);
			else
				frame << "   ....";
		}
	}

	void RenderStatusMessage(std::ostringstream& frame, GameState state)
	{
		if (state == GameState::Paused)
			frame << "\nPaused - Press P to resume, Q/Esc to quit\n";
		else if (state == GameState::GameOver)
			frame << "\nGame Over - Press R to restart, Q/Esc to quit\n";
	}
}
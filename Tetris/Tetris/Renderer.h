#pragma once

#include "Board.h"
#include "Tetromino.h"
#include "Types.h"

#include <array>
#include <sstream>

namespace Renderer
{
	bool ContainsPoint(const std::array<Point, 4>& blocks, Point point);
	void AppendColoredCell(std::ostringstream& frame, char cell);
	char GetBoardCellDisplay(const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, Point point);
	void RenderPiecePreviewRow(std::ostringstream& frame, const std::array<Point, 4>& blocks, int previewY);
	void RenderBoardRow(std::ostringstream& frame, const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, int y);
	void RenderSidePanelRow(std::ostringstream& frame, const std::array<Point, 4>& nextBlocks, const std::array<Point, 4>& holdBlocks, bool hasHoldPiece, int y);
	void RenderStatusMessage(std::ostringstream& frame, GameState state);
}
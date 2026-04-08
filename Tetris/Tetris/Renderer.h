#pragma once

#include "Board.h"
#include "Tetromino.h"
#include "Types.h"

#include <array>
#include <sstream>

// 콘솔 출력 전용 헬퍼 모음이다. Game은 화면 구성 순서만 결정하고, 실제 셀 표현은 이쪽에 맡긴다.
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
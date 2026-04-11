#pragma once

#include "Board.h"
#include "Tetromino.h"
#include "Types.h"

#include <array>
#include <sstream>

// 콘솔 출력 전용 헬퍼 모음이다.
// Game은 화면 구성 순서만 결정하고, 실제 보드/패널/상태 메시지 렌더링은 Renderer가 담당한다.
namespace Renderer
{
	// 좌표 포함 여부와 셀 표시 문자를 계산한다.
	bool ContainsPoint(const std::array<Point, 4>& blocks, Point point);
	void AppendColoredCell(std::ostringstream& frame, char cell);
	char GetBoardCellDisplay(const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, Point point);

	// 보드와 미리보기 영역의 한 줄을 렌더링한다.
	void RenderPiecePreviewRow(std::ostringstream& frame, const std::array<Point, 4>& blocks, int previewY);
	void RenderBoardRow(std::ostringstream& frame, const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, int y);
	void RenderSidePanelRow(std::ostringstream& frame, const std::array<Point, 4>& nextBlocks, const std::array<Point, 4>& holdBlocks, bool hasHoldPiece, int y);

	// 현재 게임 상태에 맞는 상태 메시지를 렌더링한다.
	void RenderStatusMessage(std::ostringstream& frame, GameState state);
}
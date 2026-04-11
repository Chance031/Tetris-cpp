#include "Renderer.h"

namespace
{
	// next/hold 블록 미리보기는 4x4 영역 기준으로 렌더링한다.
	constexpr int NextPiecePreviewSize = 4;
}

namespace Renderer
{
	// 주어진 좌표가 blocks 안에 포함되어 있는지 확인한다.
	bool ContainsPoint(const std::array<Point, 4>& blocks, Point point)
	{
		for (const Point& block : blocks)
		{
			if (block.x == point.x && block.y == point.y)
				return true;
		}

		return false;
	}

	// '@'는 현재 조작 중인 블록, '#'는 보드에 고정된 블록, '+'는 고스트 블록을 뜻한다.
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

	// 현재 좌표의 셀을 어떤 문자로 표시할지 결정한다.
	// 현재 블록, 고정 블록, 고스트 블록 순서에 따라 우선순위를 적용한다.
	char GetBoardCellDisplay(const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, Point point)
	{
		// 현재 블록이 있는 칸이면 반환한다.
		if (ContainsPoint(currentBlocks, point))
			return '@';

		// 보드에 고정된 블록이 있는 칸이면 반환한다.
		if (board.IsCellFilled(point))
			return '#';

		// 고스트 블록이 있는 칸이면 반환한다.
		if (ContainsPoint(ghostBlocks, point))
			return '+';

		// 아무 블록도 없는 빈 칸이면 반환한다.
		return '.';
	}

	// 미리보기 블록 영역의 한 줄을 렌더링한다.
	void RenderPiecePreviewRow(std::ostringstream& frame, const std::array<Point, 4>& blocks, int previewY)
	{
		// 미리보기 블록을 패널 안쪽으로 들여쓴다.
		frame << "   ";

		for (int previewX = 0; previewX < NextPiecePreviewSize; ++previewX)
		{
			Point previewPoint{ previewX, previewY };
			AppendColoredCell(frame, ContainsPoint(blocks, previewPoint) ? '@' : '.');
		}
	}

	// 보드 영역의 한 줄을 렌더링한다.
	void RenderBoardRow(std::ostringstream& frame, const Board& board, const std::array<Point, 4>& currentBlocks, const std::array<Point, 4>& ghostBlocks, int y)
	{
		for (int x = 0; x < Board::Width; ++x)
		{
			Point currentPoint{ x, y };
			AppendColoredCell(frame, GetBoardCellDisplay(board, currentBlocks, ghostBlocks, currentPoint));
		}
	}

	// 사이드 패널의 한 줄을 렌더링한다.
	// y 값에 따라 next/hold 제목과 미리보기 영역을 해당 위치에 배치한다.
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

	// 일시 정지나 게임 오버 상태일 때 조작 안내가 포함된 상태 메시지를 출력한다.
	void RenderStatusMessage(std::ostringstream& frame, GameState state)
	{
		if (state == GameState::Paused)
			frame << "\nPaused - Press P to resume, Q/Esc to quit\n";
		else if (state == GameState::GameOver)
			frame << "\nGame Over - Press R to restart, Q/Esc to quit\n";
	}
}
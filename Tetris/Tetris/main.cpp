#include "Board.h"

#include <iostream>

int main()
{
	Board board;
	Tetromino piece_1(TetrominoType::T);

	piece_1.SetPosition(4, 0);

	if (board.CanPlace(piece_1))
	{
		board.LockPiece(piece_1);
		std::cout << "Piece locked.\n";
	}

	Tetromino piece_2(TetrominoType::I);
	
	piece_2.SetPosition(4, 0);

	if (board.CanPlace(piece_2))
	{
		board.LockPiece(piece_2);
		std::cout << "Piece locked.\n";
	}
	else
		std::cout << "이미 블록이 존재합니다.\n";

	const int clearedLines = board.ClearLines();
	std::cout << "Cleared Lines: " << clearedLines << '\n';

	return 0;
}
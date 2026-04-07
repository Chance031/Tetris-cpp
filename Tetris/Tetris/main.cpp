#include "Board.h"

#include <iostream>

int main()
{
	Board board;
	Tetromino piece(TetrominoType::T);

	piece.SetPosition(4, 0);

	if (board.CanPlace(piece))
	{
		board.LockPiece(piece);
		std::cout << "Piece locked.\n";
	}

	const int clearedLines = board.ClearLines();
	std::cout << "Cleared Lines: " << clearedLines << '\n';

	return 0;
}
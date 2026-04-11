#pragma once

#include "Types.h"

#include <array>

// 현재 조작 중인 테트리스 블록 1개를 표현한다.
//
// Tetromino는 블록의 종류, 기준 위치, 회전 상태만 관리한다.
// 충돌 판정, 고정, 줄 삭제 같은 게임 규칙은 Board/Game의 책임이다.
class Tetromino
{
public:
	Tetromino() = default;
	explicit Tetromino(TetrominoType type);

	// 블록 종류와 상태를 설정한다.
	void SetType(TetrominoType newType);
	void SetPosition(int x, int y);
	void SetRotation(int newRotation);

	// 블록을 이동하거나 회전한다.
	void Move(int dx, int dy);
	void RotateCW();
	void RotateCCW();

	// 현재 블록 상태를 조회한다.
	std::array<Point, 4> GetBlockLocations() const;
	TetrominoType GetType() const;
	Point GetPosition() const;
	int GetRotation() const;

private:
	TetrominoType m_type = TetrominoType::I;
	Point m_position = { 0, 0 };

	// 내부에서는 0~3 회전 인덱스로 저장한다.
	int m_rotation = 0;
};

// SRS 벽킥 후보를 반환한다.
// Game은 반환된 후보를 순서대로 적용해 회전을 시도한다.
std::array<Point, 5> GetSrsKicks(TetrominoType type, int oldRotationIndex, RotationDirection direction);
#pragma once

#include "Types.h"

#include <array>

// 현재 조작 중인 테트리스 블록 1개를 표현한다.
//
// Tetromino는 블록의 종류, 기준 위치, 회전 상태만 관리한다.
// 충돌 판정과 게임 로직은 Board/Game의 책임이다.
class Tetromino
{
public:
	Tetromino() = default;
	explicit Tetromino(TetrominoType type);

	void SetType(TetrominoType newType);
	void SetPosition(int x, int y);
	void SetRotation(int newRotation);

	void Move(int dx, int dy);
	void RotateCW();
	void RotateCCW();

	std::array<Point, 4> GetBlockLocations() const;
	TetrominoType GetType() const;
	Point GetPosition() const;
	int GetRotation() const;

private:
	TetrominoType m_type = TetrominoType::I;
	Point m_position = { 0, 0 };

	// 내부에서는 0~3 회전 인덱스로 저장하고, 외부에는 각도(0, 90, 180, 270)로 반환한다.
	int m_rotation = 0;
};

std::array<Point, 5> GetSrsKicks(TetrominoType type, int oldRotationIndex, RotationDirection direction);

#include "Tetromino.h"

namespace
{
	// [블록 종류][회전 인덱스][y][x] 순서의 4x4 모양 데이터다.
	// 블록 종류는 TetrominoType의 선언 순서(I, J, L, O, S, T, Z)를 따른다.
	// 회전 인덱스는 0, 1, 2, 3이 각각 0도, 90도, 180도, 270도를 뜻한다.
	constexpr int SHAPE_DATA[7][4][4][4] =
	{
		// I
		{
			{
				{ 0, 0, 0, 0 },
				{ 1, 1, 1, 1 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 1, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 1, 0 }
			},
			{
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 },
				{ 1, 1, 1, 1 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 }
			}
		},
		// J
		{
			{
				{ 1, 0, 0, 0 },
				{ 1, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 1, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 0, 0 },
				{ 1, 1, 1, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 1, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			}
		},
		// L
		{
			{
				{ 0, 0, 1, 0 },
				{ 1, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 0, 0 },
				{ 1, 1, 1, 0 },
				{ 1, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 1, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			}
		},
		// O
		{
			{
				{ 0, 1, 1, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 1, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 1, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 1, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			}
		},
		// S
		{
			{
				{ 0, 1, 1, 0 },
				{ 1, 1, 0, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 1, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 0, 0 },
				{ 0, 1, 1, 0 },
				{ 1, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 1, 0, 0, 0 },
				{ 1, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			}
		},
		// T
		{
			{
				{ 0, 1, 0, 0 },
				{ 1, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 0, 0 },
				{ 1, 1, 1, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 1, 1, 0, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			}
		},
		// Z
		{
			{
				{ 1, 1, 0, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 1, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 1, 0, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 0, 0, 0 },
				{ 1, 1, 0, 0 },
				{ 0, 1, 1, 0 },
				{ 0, 0, 0, 0 }
			},
			{
				{ 0, 1, 0, 0 },
				{ 1, 1, 0, 0 },
				{ 1, 0, 0, 0 },
				{ 0, 0, 0, 0 }
			}
		}
	};
}

std::array<Point, 5> GetSrsKicks(TetrominoType type, int oldRotationIndex, RotationDirection direction)
{
	const bool isClockwise = direction == RotationDirection::Clockwise;

	if (type == TetrominoType::I)
	{
		switch (oldRotationIndex)
		{
		case 0:
			return isClockwise
				? std::array<Point, 5>{ Point{ 0, 0 }, Point{ -2, 0 }, Point{ 1, 0 }, Point{ -2, 1 }, Point{ 1, -2 } }
				: std::array<Point, 5>{ Point{ 0, 0 }, Point{ -1, 0 }, Point{ 2, 0 }, Point{ -1, -2 }, Point{ 2, 1 } };
		case 1:
			return isClockwise
				? std::array<Point, 5>{ Point{ 0, 0 }, Point{ -1, 0 }, Point{ 2, 0 }, Point{ -1, -2 }, Point{ 2, 1 } }
				: std::array<Point, 5>{ Point{ 0, 0 }, Point{ -2, 0 }, Point{ 1, 0 }, Point{ -2, 1 }, Point{ 1, -2 } };
		case 2:
			return isClockwise
				? std::array<Point, 5>{ Point{ 0, 0 }, Point{ 2, 0 }, Point{ -1, 0 }, Point{ 2, -1 }, Point{ -1, 2 } }
				: std::array<Point, 5>{ Point{ 0, 0 }, Point{ 1, 0 }, Point{ -2, 0 }, Point{ 1, 2 }, Point{ -2, -1 } };
		case 3:
			return isClockwise
				? std::array<Point, 5>{ Point{ 0, 0 }, Point{ 1, 0 }, Point{ -2, 0 }, Point{ 1, 2 }, Point{ -2, -1 } }
				: std::array<Point, 5>{ Point{ 0, 0 }, Point{ 2, 0 }, Point{ -1, 0 }, Point{ 2, -1 }, Point{ -1, 2 } };
		}
	}

	switch (oldRotationIndex)
	{
	case 0:
		return isClockwise
			? std::array<Point, 5>{ Point{ 0, 0 }, Point{ -1, 0 }, Point{ -1, -1 }, Point{ 0, 2 }, Point{ -1, 2 } }
			: std::array<Point, 5>{ Point{ 0, 0 }, Point{ 1, 0 }, Point{ 1, -1 }, Point{ 0, 2 }, Point{ 1, 2 } };
	case 1:
		return isClockwise
			? std::array<Point, 5>{ Point{ 0, 0 }, Point{ 1, 0 }, Point{ 1, 1 }, Point{ 0, -2 }, Point{ 1, -2 } }
			: std::array<Point, 5>{ Point{ 0, 0 }, Point{ -1, 0 }, Point{ -1, 1 }, Point{ 0, -2 }, Point{ -1, -2 } };
	case 2:
		return isClockwise
			? std::array<Point, 5>{ Point{ 0, 0 }, Point{ 1, 0 }, Point{ 1, -1 }, Point{ 0, 2 }, Point{ 1, 2 } }
			: std::array<Point, 5>{ Point{ 0, 0 }, Point{ -1, 0 }, Point{ -1, -1 }, Point{ 0, 2 }, Point{ -1, 2 } };
	case 3:
		return isClockwise
			? std::array<Point, 5>{ Point{ 0, 0 }, Point{ -1, 0 }, Point{ -1, 1 }, Point{ 0, -2 }, Point{ -1, -2 } }
			: std::array<Point, 5>{ Point{ 0, 0 }, Point{ 1, 0 }, Point{ 1, 1 }, Point{ 0, -2 }, Point{ 1, -2 } };
	default:
		return {};
	}
}

Tetromino::Tetromino(TetrominoType type) : m_type(type)
{
}

void Tetromino::SetType(TetrominoType newType)
{
	m_type = newType;
}

void Tetromino::SetPosition(int x, int y)
{
	m_position.x = x;
	m_position.y = y;
}

// 외부에서 각도로 받은 값을 내부 회전 인덱스(0~3)로 정규화한다.
void Tetromino::SetRotation(int newRotation)
{
	int normalized = newRotation % 360;

	if (normalized < 0)
		normalized += 360;

	m_rotation = (normalized / 90) % 4;
}

void Tetromino::Move(int dx, int dy)
{
	m_position.x += dx;
	m_position.y += dy;
}

void Tetromino::RotateCW()
{
	m_rotation = (m_rotation + 1) % 4;
}

void Tetromino::RotateCCW()
{
	m_rotation = (m_rotation + 3) % 4;
}

// 현재 블록이 차지하는 4칸을 보드 좌표로 변환해서 반환한다.
std::array<Point, 4> Tetromino::GetBlockLocations() const
{
	const int typeIndex = static_cast<int>(m_type);
	const int rotationIndex = m_rotation;

	std::array<Point, 4> blockLocations{};
	int blockIndex = 0;

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			if (SHAPE_DATA[typeIndex][rotationIndex][y][x] == 0)
				continue;

			blockLocations[blockIndex] = { m_position.x + x, m_position.y + y };
			++blockIndex;
		}
	}

	return blockLocations;
}

TetrominoType Tetromino::GetType() const
{
	return m_type;
}

Point Tetromino::GetPosition() const
{
	return m_position;
}

int Tetromino::GetRotation() const
{
	return m_rotation * 90;
}

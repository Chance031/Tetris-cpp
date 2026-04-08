#pragma once

// 프로젝트 전반에서 공유하는 가벼운 기본 타입들이다.
// Game, Board 같은 무거운 클래스 정의나 구현 코드는 이 파일에 두지 않는다.

// 보드의 한 칸 좌표를 표현한다.
struct Point
{
    int x = 0;
    int y = 0;
};

// 게임의 큰 진행 상태를 표현한다.
enum class GameState
{
    Title,
    Playing,
    Paused,
    GameOver,
    Exit
};

// 테트리스의 7종 블록 타입이다.
// SHAPE_DATA의 블록 순서와 맞춰 사용하므로 선언 순서를 바꿀 때는 함께 수정해야 한다.
enum class TetrominoType
{
    I,
    J,
    L,
    O,
    S,
    T,
    Z
};

// 블록 회전 방향을 표현한다.
enum class RotationDirection
{
    Clockwise,
    CounterClockwise
};

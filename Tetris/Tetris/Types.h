#pragma once

// 프로젝트 전반에서 공유하는 가벼운 기본 타입들이다.
//
// Game, Board 같은 무거운 클래스의 정의나 구현은 이 파일에 두지 않는다.

// 보드의 한 칸을 가리키는 정수 좌표이다.
struct Point
{
    int x = 0;
    int y = 0;
};

// 게임의 상위 진행 상태를 표현한다.
enum class GameState
{
    Title,      // 타이틀 화면
    Playing,    // 게임 진행 중
    Paused,     // 일시 정지
    GameOver,   // 게임 오버
    Exit        // 게임 종료
};

// 테트리스의 7종 블록 타입이다.
//
// SHAPE_DATA의 블록 순서와 1:1로 대응하므로, 선언 순서를 바꾸면 관련 데이터도 함께 수정해야 한다.
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

// 블록 회전에 사용하는 방향이다.
enum class RotationDirection
{
    Clockwise,          // 시계 방향(CW)
    CounterClockwise    // 반시계 방향(CCW)
};
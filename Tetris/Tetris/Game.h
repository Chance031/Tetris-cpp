#pragma once

#include "Board.h"
#include "Tetromino.h"
#include "Types.h"

#include <chrono>
#include <iosfwd>
#include <random>
#include <string>
#include <vector>

// 한 번의 게임 결과를 하이스코어 표에 저장하기 위한 값 객체다.
struct HighScoreEntry
{
	std::string name;
	int score = 0;
};

// Game 루프 전체를 조율하는 최상위 클래스다.
//
// 입력 처리, 상태 업데이트, 렌더링 순서를 관리하고
// Board와 Tetromino의 상호작용을 중재한다.
class Game
{
public:
	Game();

	// 게임 시작 전 초기화와 메인 루프 실행을 담당한다.
	void Initialize();
	void Run();

private:
	// 세션 시작과 상태 전환을 처리한다.
	void StartNewSession();
	void TransitionTo(GameState newState);

	// 상태별 입력 처리와 프레임 진행을 담당한다.
	void HandleInput();
	void HandleTitleInput();
	void HandlePausedInput();
	void HandleGameOverInput();
	void Update();
	void Render();

	// 하이스코어 입출력과 렌더링을 처리한다.
	void PromptAndSaveHighScore();
	void AddHighScore(const std::string& name, int score);
	void RenderHighScores(std::ostringstream& frame) const;
	void SaveHighScores() const;
	void LoadHighScores();

	// 점수와 레벨 계산을 처리한다.
	int CalculateScore(int clearedLines) const;
	int CalculateTSpinScore(int clearedLines) const;
	void UpdateLevel();

	// 블록 생성, 이동, 회전, 고정, 홀드를 처리한다.
	void SpawnNextPiece();
	void ProcessLockAndResolve();
	bool TryMoveCurrentPiece(int dx, int dy, bool lockOnFail);
	bool TryRotateCurrentPiece(RotationDirection direction);
	void HardDropCurrentPiece();
	void HoldCurrentPiece();
	TetrominoType CreateRandomTetrominoType();
	void RefillPieceBag();

	// 고스트 피스, 락 딜레이, T-Spin 판정을 처리한다.
	void StartLockDelay();
	void ResetLockDelay();
	void RefreshLockDelayAfterSuccessfulMove();
	bool IsCurrentPieceTouchingGround() const;
	bool DetectTSpin() const;
	Tetromino GetGhostPiece() const;

private:
	// 낙하 속도와 고정 지연 관련 규칙이다.
	static constexpr int InitialFallIntervalMs = 800;
	static constexpr int FallIntervalDecreasePerLevel = 50;
	static constexpr int MinFallIntervalMs = 100;
	static constexpr int LockDelayMs = 500;
	static constexpr int MaxLockResetCount = 15;

	// 점수 규칙은 Game에 모아 두어 라인 정산 흐름에서 한눈에 확인한다.
	static constexpr int SoftDropScorePerCell = 1;
	static constexpr int ComboScorePerStep = 50;
	static constexpr int TSpinNoLineScore = 400;
	static constexpr int TSpinSingleScore = 800;
	static constexpr int TSpinDoubleScore = 1200;
	static constexpr int TSpinTripleScore = 1600;

	// 저장 가능한 하이스코어와 이름 길이 제한이다.
	static constexpr int MaxHighScoreCount = 5;
	static constexpr int MaxPlayerNameLength = 12;

	// 현재 게임 진행에 필요한 핵심 상태다.
	Board m_board;
	Tetromino m_currentPiece;
	Tetromino m_nextPiece;
	Tetromino m_holdPiece;

	GameState m_state = GameState::Title;
	bool m_isLockRequired = false; // true이면 다음 Update()에서 현재 블록을 보드에 고정한다.

	int m_score = 0;
	int m_level = 1;
	int m_totalLines = 0;
	int m_combo = -1;                   // -1은 콤보가 없는 상태를 뜻한다. 줄을 클리어할 때마다 1씩 증가한다.
	bool m_isBackToBackActive = false;  // 어려운 클리어(테트리스, T-Spin)가 연속으로 이어지는 상태를 뜻한다.
	std::string m_lastClearMessage;     // 가장 최근 클리어 종류를 화면에 표시하기 위한 문자열이다.
	bool m_needsHighScoreName = false;  // true이면 게임 오버 후 이름 입력 절차를 시작한다.

	// 시간 기반 낙하와 고정 지연을 위한 상태다.
	std::chrono::steady_clock::time_point m_lastFallTime;
	std::chrono::milliseconds m_fallInterval{ InitialFallIntervalMs };
	std::chrono::steady_clock::time_point m_lockStartTime;
	std::chrono::milliseconds m_lockDelay{ LockDelayMs };
	bool m_isTouchingGround = false;
	int m_lockResetCount = 0;
	bool m_lastMoveWasRotation = false;

	// 홀드, 랜덤 가방, 하이스코어 저장 상태다.
	bool m_hasHoldPiece = false;
	bool m_canHold = true;

	std::mt19937 m_randomEngine;
	std::vector<TetrominoType> m_pieceBag;
	std::vector<HighScoreEntry> m_highScores;
};


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
// 입력 처리, 상태 업데이트, 렌더링 순서를 관리하고
// Board와 Tetromino의 상호작용을 중재한다.
class Game
{
public:
	Game();

	void Initialize();
	void Run();

private:
	void StartNewSession();

	void HandleInput();
	void HandleTitleInput();
	void HandlePausedInput();
	void HandleGameOverInput();
	void Update();
	void Render();

	void PromptAndSaveHighScore();
	void AddHighScore(const std::string& name, int score);
	void RenderHighScores(std::ostringstream& frame) const;

	// 게임오버 후 점수를 기록하고, 실행 간 유지되도록 scores.txt에 저장한다.
	void SaveHighScores() const;
	void LoadHighScores();

	// 상태 변경 지점을 한 곳으로 모아 이후 전환 부작용을 붙이기 쉽게 한다.
	void TransitionTo(GameState newState);

	int CalculateScore(int clearedLines) const;
	int CalculateTSpinScore(int clearedLines) const;
	void UpdateLevel();

	void SpawnNextPiece();
	void ProcessLockAndResolve();

	bool TryMoveCurrentPiece(int dx, int dy, bool lockOnFail);
	bool TryRotateCurrentPiece(RotationDirection direction);
	void StartLockDelay();
	void ResetLockDelay();
	void RefreshLockDelayAfterSuccessfulMove();
	bool IsCurrentPieceTouchingGround() const;
	bool DetectTSpin() const;
	void HardDropCurrentPiece();
	void HoldCurrentPiece();
	TetrominoType CreateRandomTetrominoType();
	void RefillPieceBag();

	Tetromino GetGhostPiece() const;

private:
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
	static constexpr int MaxHighScoreCount = 5;
	static constexpr int MaxPlayerNameLength = 12;

	Board m_board;
	Tetromino m_currentPiece;
	Tetromino m_nextPiece;
	Tetromino m_holdPiece;

	GameState m_state = GameState::Title;
	bool m_isLockRequired = false; // true이면 다음 Update()에서 현재 블록을 보드에 고정한다.

	int m_score = 0;
	int m_level = 1;
	int m_totalLines = 0;
	int m_combo = -1;
	bool m_isBackToBackActive = false;
	std::string m_lastClearMessage;
	bool m_needsHighScoreName = false;

	std::chrono::steady_clock::time_point m_lastFallTime;
	std::chrono::milliseconds m_fallInterval{ InitialFallIntervalMs };
	std::chrono::steady_clock::time_point m_lockStartTime;
	std::chrono::milliseconds m_lockDelay{ LockDelayMs };
	bool m_isTouchingGround = false;
	int m_lockResetCount = 0;
	bool m_lastMoveWasRotation = false;

	bool m_hasHoldPiece = false;
	bool m_canHold = true;

	std::mt19937 m_randomEngine;
	std::vector<TetrominoType> m_pieceBag;
	std::vector<HighScoreEntry> m_highScores;
};


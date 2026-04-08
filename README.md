# 🎮 Tetris-cpp

C++로 만드는 콘솔 테트리스 프로젝트

## 🛠️ 기술 스택

![C++](https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white)

## 🤖 사용 AI

![Claude](https://img.shields.io/badge/Claude-D97757?style=flat&logo=anthropic&logoColor=white)
![ChatGPT](https://img.shields.io/badge/ChatGPT-74aa9c?style=flat&logo=openai&logoColor=white)

---

## 📐 설계 문서

| 문서 | 설명 |
|------|------|
| [📊 플로우차트 (draw.io로 열기)](https://app.diagrams.net/?url=https://raw.githubusercontent.com/Chance031/Tetris-cpp/main/Tetris.drawio) | 게임 전체 상태 흐름, 플레이 루프, 입력 처리, 충돌 판정, 정산 시퀀스, UML (6페이지) |

---

## 📅 개발 일지

<!-- GANTT_START -->
```mermaid
gantt
    title 테트리스 개발 일정
    dateFormat YYYY-MM-DD
    section 시작
        프로젝트 초기화 :done, init, 2026-04-07, 1d
    section Tetromino
        Tetromino .h / .cpp 구현 :done, t16803, 2026-04-07, 1d
    section Board
        Board .h / .cpp 구현 :done, t84048, 2026-04-07, 1d
    section 리팩토링
        코드 리뷰 반영 및 리팩토링 :done, refactor, 2026-04-07, 1d
    section Game
        Game .h / .cpp 구현 :done, t91896, 2026-04-08, 1d
    section Render / Update
        렌더링 및 업데이트 구현 :done, t47867, 2026-04-08, 1d
    section Input
        키보드 입력 구현 및 수정 :done, t8258, 2026-04-08, 1d
    section Level / Score
        레벨업, 낙하속도, 점수, 다음블록 초기화 :done, t44367, 2026-04-08, 1d
    section UI / GameOver
        미리보기 위치 변경, 게임오버 후 재시작 기능 :done, t55123, 2026-04-08, 1d
    section 품질 개선
        매직 넘버 상수화, Wall Kick, UpdateLevel 단순화 :done, t66234, 2026-04-08, 1d
    section FallInterval
        낙하 속도 매직 넘버 상수화 :active, t66782, 2026-04-08, 1d
    section MagicNumber
        매직 넘버 상수화 :active, t23180, 2026-04-08, 1d
    section Title
        Init Title :done, t45140, 2026-04-08, 1d
    section GameOver
        게임오버 후 재시작 시 화면 잔여 메시지 제거 :crit, t96693, 2026-04-08, 1d
    section GhostPiece
        Init GhostPiece :done, t88826, 2026-04-08, 1d
    section 7-bag
        랜덤 7-bag Init :done, t21893, 2026-04-08, 1d
    section Hold
        Init Hold :done, t97044, 2026-04-08, 1d
    section Pause
        Init Pause :done, t97166, 2026-04-08, 1d
    section soft drop
        소프트 드롭 점수 :done, t4875, 2026-04-08, 1d
    section UI
        UI 개선 :crit, t85460, 2026-04-08, 1d
    section Combo
        콤보 및 B2B 기능 추가 :done, t36199, 2026-04-08, 1d
    section T-Spin
        T-스핀 :done, t94811, 2026-04-08, 1d
```
<!-- GANTT_END -->

---

## ✅ 구현 현황

<!-- CHECKLIST_START -->
- [x] 충돌 처리
- [x] 라인 클리어
- [x] 블록 생성 및 렌더링
- [x] 블록 이동 / 회전
- [x] 점수 시스템
- [x] 게임 오버 / 재시작
- [x] 레벨 / 낙하 속도
- [x] Wall Kick
- [ ] 맵 렌더링
<!-- CHECKLIST_END -->

---

## 📝 작업 로그

<!-- LOG_START -->
| 날짜 | 섹션 | 작업 내용 |
|------|------|-----------|
| 2026-04-07 | feat | 프로젝트 초기화 |
| 2026-04-07 | feat | Tetromino .h / .cpp 구현 |
| 2026-04-07 | feat | Board .h / .cpp 구현 |
| 2026-04-07 | refactor | m_rotation 인덱스화, SHAPE_DATA 익명 네임스페이스, Rule of Zero, 주석 개선 |
| 2026-04-07 | refactor | Board::Reset() memset 적용, 스폰 위치 상수화, #include 중복 제거 |
| 2026-04-07 | test | LockPiece / CanPlace / ClearLines 단위 동작 확인 |
| 2026-04-07 | chore | .gitignore 설정, GitHub 초기 push 완료 |
| 2026-04-08 | feat | Game .h / .cpp 구현 |
| 2026-04-08 | feat | Game::Update() / Render() 기본 구현 |
| 2026-04-08 | feat | 키보드 입력 처리 구현 |
| 2026-04-08 | fix | 키 입력 처리 수정 |
| 2026-04-08 | feat | 레벨업, 낙하 속도, 점수 업데이트, 다음 블록 초기화 구현 |
| 2026-04-08 | feat | 미리보기 블록 위치 우측 상단으로 이동 |
| 2026-04-08 | feat | 게임오버 후 종료 / 재시작 선택 기능 추가 |
| 2026-04-08 | refactor | 낙하 속도 매직 넘버 상수화 |
| 2026-04-08 | feat | TryRotateCurrentPieceCW에 Wall Kick 추가 |
| 2026-04-08 | refactor | UpdateLevel() std::max로 단순화 |
| 2026-04-08 | FallInterval | 낙하 속도 매직 넘버 상수화 |
| 2026-04-08 | MagicNumber | 매직 넘버 상수화 |
| 2026-04-08 | Title | Init Title |
| 2026-04-08 | GameOver | 게임오버 후 재시작 시 화면 잔여 메시지 제거 |
| 2026-04-08 | GhostPiece | Init GhostPiece |
| 2026-04-08 | 7-bag | 랜덤 7-bag Init |
| 2026-04-08 | Hold | Init Hold |
| 2026-04-08 | Pause | Init Pause |
| 2026-04-08 | soft drop | 소프트 드롭 점수 |
| 2026-04-08 | UI | UI 개선 |
| 2026-04-08 | Combo | 콤보 및 B2B 기능 추가 |
| 2026-04-08 | T-Spin | T-스핀 |
<!-- LOG_END -->

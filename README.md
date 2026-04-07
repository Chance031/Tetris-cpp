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
 section tetromino
 테트로미노 .h / .cpp 생성 :done, t16803, 2026-04-07, 1d
 section Init Board
 보드 생성 :done, t84048, 2026-04-07, 1d
 section 리팩토링
 코드 리뷰 반영 및 리팩토링 :done, refactor, 2026-04-07, 1d
 section 테스트
 단위 테스트 (Board + Tetromino) :done, test, 2026-04-07, 1d
 section Init Game
 게임 .h / .cpp 생성 :done, t91896, 2026-04-08, 1d
 section Render / Update
 렌더링 및 업데이트 구현 :done, t47867, 2026-04-08, 1d
 section Input
 키보드 입력 구현 및 수정 :done, t8258, 2026-04-08, 1d
```
<!-- GANTT_END -->

---

## ✅ 구현 현황

<!-- CHECKLIST_START -->
- [x] 충돌 처리 (단위 테스트 확인)
- [x] 라인 클리어 (단위 테스트 확인)
- [x] 블록 생성 및 렌더링
- [x] 블록 이동 / 회전
- [ ] 맵 렌더링
- [ ] 점수 시스템
- [ ] 게임 오버 / 재시작
<!-- CHECKLIST_END -->

---

## 📝 작업 로그

<!-- LOG_START -->
| 날짜 | 타입 | 작업 내용 |
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
<!-- LOG_END -->

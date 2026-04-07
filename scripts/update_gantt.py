"""
커밋 메시지를 파싱해서 README의 Mermaid 간트 차트 + 작업 로그를 자동 업데이트하는 스크립트.

커밋 메시지 형식:
    feat(rendering): 맵 렌더링 구현
    fix(logic): 블록 충돌 버그 수정
    refactor(board): Board 클래스 분리
    docs(readme): 주석 추가
"""

import subprocess
import re
import os
from datetime import datetime, timezone, timedelta

README_PATH = "README.md"

SECTION_STATUS = {
    "feat":     "done",
    "fix":      "crit",
    "refactor": "active",
    "docs":     None,
    "chore":    None,
}

FEATURE_KEYWORDS = {
    "맵 렌더링":         "맵 렌더링",
    "블록 생성":         "블록 생성 및 렌더링",
    "블록 렌더링":       "블록 생성 및 렌더링",
    "블록 이동":         "블록 이동 / 회전",
    "블록 회전":         "블록 이동 / 회전",
    "충돌":              "충돌 처리",
    "라인 클리어":       "라인 클리어",
    "점수":              "점수 시스템",
    "게임 오버":         "게임 오버 / 재시작",
    "재시작":            "게임 오버 / 재시작",
}

KST = timezone(timedelta(hours=9))


def get_latest_commits():
    before = os.environ.get("BEFORE_SHA", "")
    after  = os.environ.get("GITHUB_SHA", "HEAD")

    if before and before != "0000000000000000000000000000000000000000":
        log_range = f"{before}..{after}"
    else:
        log_range = "-1"

    result = subprocess.run(
        ["git", "log", log_range, "--pretty=format:%s"],
        capture_output=True, text=True
    )
    return result.stdout.strip().splitlines()


def parse_commit(message: str):
    pattern = r"^(\w+)\(([^)]+)\):\s*(.+)$"
    m = re.match(pattern, message.strip())
    if not m:
        return None
    commit_type, section, description = m.groups()
    if commit_type not in SECTION_STATUS:
        return None
    return commit_type, section, description


def read_readme():
    with open(README_PATH, "r", encoding="utf-8") as f:
        return f.read()

def write_readme(content: str):
    with open(README_PATH, "w", encoding="utf-8") as f:
        f.write(content)

def replace_block(content: str, start_tag: str, end_tag: str, new_block: str) -> str:
    pattern = re.compile(
        rf"({re.escape(start_tag)}\n).*?(\n{re.escape(end_tag)})",
        re.DOTALL
    )
    return pattern.sub(rf"\1{new_block}\2", content)


def parse_existing_gantt(content: str):
    block_match = re.search(
        r"<!-- GANTT_START -->\n```mermaid\ngantt\n(.*?)```\n<!-- GANTT_END -->",
        content, re.DOTALL
    )
    if not block_match:
        return {}

    sections = {}
    current_section = "기타"

    for line in block_match.group(1).splitlines():
        line = line.strip()
        sec_m = re.match(r"section (.+)", line)
        if sec_m:
            current_section = sec_m.group(1)
            sections.setdefault(current_section, [])
            continue

        task_m = re.match(r"(.+?)\s*:(.*)", line)
        if task_m:
            desc = task_m.group(1).strip()
            meta = [x.strip() for x in task_m.group(2).split(",")]
            sections.setdefault(current_section, []).append({
                "description": desc,
                "meta": meta
            })

    return sections


def build_gantt(sections: dict) -> str:
    lines = [
        "```mermaid",
        "gantt",
        "    title 테트리스 개발 일정",
        "    dateFormat YYYY-MM-DD",
    ]
    for section, tasks in sections.items():
        lines.append(f"    section {section}")
        for task in tasks:
            meta_str = ", ".join(task["meta"])
            lines.append(f"        {task['description']} :{meta_str}")
    lines.append("```")
    return "\n".join(lines)


def add_to_gantt(sections: dict, commit_type: str, section: str, description: str, date_str: str):
    sections.setdefault(section, [])
    for task in sections[section]:
        if task["description"] == description:
            return
    status = SECTION_STATUS.get(commit_type, "done")
    alias = f"t{abs(hash(description)) % 100000}"
    meta = [status, alias, date_str, "1d"] if status else [alias, date_str, "1d"]
    sections[section].append({"description": description, "meta": meta})


def update_checklist(content: str, description: str) -> str:
    for keyword, feature in FEATURE_KEYWORDS.items():
        if keyword in description:
            content = content.replace(f"- [ ] {feature}", f"- [x] {feature}")
    return content


def parse_existing_log(content: str):
    log_match = re.search(
        r"<!-- LOG_START -->\n(.*?)\n<!-- LOG_END -->",
        content, re.DOTALL
    )
    if not log_match:
        return []
    rows = []
    for line in log_match.group(1).splitlines():
        if line.startswith("| ") and "---" not in line and "날짜" not in line:
            rows.append(line)
    return rows


def build_log(rows: list) -> str:
    header = "| 날짜 | 섹션 | 작업 내용 |\n|------|------|-----------|"
    return header + "\n" + "\n".join(rows)


def main():
    commits = get_latest_commits()
    if not commits:
        print("파싱할 커밋 없음. 종료.")
        return

    today = datetime.now(KST).strftime("%Y-%m-%d")
    content = read_readme()
    sections = parse_existing_gantt(content)
    log_rows = parse_existing_log(content)
    changed = False

    for msg in commits:
        parsed = parse_commit(msg)
        if not parsed:
            print(f"  skip: {msg}")
            continue

        commit_type, section, description = parsed
        print(f"  ✅ {commit_type}({section}): {description}")

        add_to_gantt(sections, commit_type, section, description, today)
        content = update_checklist(content, description)
        log_rows.append(f"| {today} | {section} | {description} |")
        changed = True

    if not changed:
        print("업데이트할 내용 없음.")
        return

    new_gantt = build_gantt(sections)
    content = replace_block(content, "<!-- GANTT_START -->", "<!-- GANTT_END -->", new_gantt)

    new_log = build_log(log_rows)
    content = replace_block(content, "<!-- LOG_START -->", "<!-- LOG_END -->", new_log)

    write_readme(content)
    print("README 업데이트 완료!")


if __name__ == "__main__":
    main()

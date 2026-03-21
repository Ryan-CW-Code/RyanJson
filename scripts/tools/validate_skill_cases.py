#!/usr/bin/env python3
"""Validate deterministic skill-routing eval cases for RyanJson."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
SKILLS_ROOT = REPO_ROOT / "skills"
EVAL_CASES_ROOT = SKILLS_ROOT / "evals" / "cases"
CASE_ID_RE = re.compile(r"^[a-z0-9-]{3,64}$")
MIN_CASES_BY_SKILL = {
    "ryanjson-project-guide": 4,
    "ryanjson-api-usage": 12,
    "ryanjson-optimization": 4,
    "ryanjson-test-engineering": 4,
}


class CaseValidationError(RuntimeError):
    """Raised when eval cases are invalid."""


def load_json(path: Path) -> object:
    with path.open("r", encoding="utf-8") as fh:
        return json.load(fh)


def iter_skill_names() -> list[str]:
    return sorted(path.name for path in SKILLS_ROOT.iterdir() if path.is_dir() and (path / "SKILL.md").exists())


def require_string(field_name: str, value: object, file_path: Path, case_id: str | None = None) -> str:
    if not isinstance(value, str) or not value.strip():
        prefix = f"{file_path}"
        if case_id is not None:
            prefix += f" case `{case_id}`"
        raise CaseValidationError(f"{prefix}: field `{field_name}` must be a non-empty string")
    return value.strip()


def require_string_list(field_name: str, value: object, file_path: Path, case_id: str) -> list[str]:
    if not isinstance(value, list) or not value:
        raise CaseValidationError(f"{file_path} case `{case_id}`: field `{field_name}` must be a non-empty list")

    items: list[str] = []
    for item in value:
        if not isinstance(item, str) or not item.strip():
            raise CaseValidationError(f"{file_path} case `{case_id}`: field `{field_name}` contains an invalid entry")
        items.append(item.strip())
    return items


def validate_case_path(ref: str, file_path: Path, case_id: str) -> None:
    if ref.startswith("/") or ref.startswith("./") or ref.startswith("../"):
        raise CaseValidationError(
            f"{file_path} case `{case_id}`: `must_reference` must use repo-relative paths, got `{ref}`"
        )

    target = (REPO_ROOT / ref).resolve()
    if not target.exists():
        raise CaseValidationError(f"{file_path} case `{case_id}`: missing referenced path `{ref}`")


def validate_case(
    case: object,
    file_path: Path,
    suite_skill: str,
    skill_names: set[str],
    seen_ids: set[str],
    seen_queries: set[str],
) -> None:
    if not isinstance(case, dict):
        raise CaseValidationError(f"{file_path}: each case must be an object")

    case_id = require_string("id", case.get("id"), file_path)
    if not CASE_ID_RE.fullmatch(case_id):
        raise CaseValidationError(f"{file_path} case `{case_id}`: invalid `id` format")
    if case_id in seen_ids:
        raise CaseValidationError(f"{file_path} case `{case_id}`: duplicate case id")
    seen_ids.add(case_id)

    query = require_string("query", case.get("query"), file_path, case_id)
    normalized_query = " ".join(query.split())
    if normalized_query in seen_queries:
        raise CaseValidationError(f"{file_path} case `{case_id}`: duplicate query")
    seen_queries.add(normalized_query)

    expected_skill = require_string("expected_skill", case.get("expected_skill"), file_path, case_id)
    if expected_skill not in skill_names:
        raise CaseValidationError(f"{file_path} case `{case_id}`: unknown `expected_skill` `{expected_skill}`")
    if expected_skill != suite_skill:
        raise CaseValidationError(
            f"{file_path} case `{case_id}`: `expected_skill` must match suite `{suite_skill}`"
        )

    must_reference = require_string_list("must_reference", case.get("must_reference"), file_path, case_id)
    for ref in must_reference:
        validate_case_path(ref, file_path, case_id)

    must_not_route_to = require_string_list("must_not_route_to", case.get("must_not_route_to"), file_path, case_id)
    for blocked_skill in must_not_route_to:
        if blocked_skill not in skill_names:
            raise CaseValidationError(
                f"{file_path} case `{case_id}`: unknown blocked skill `{blocked_skill}` in `must_not_route_to`"
            )
        if blocked_skill == expected_skill:
            raise CaseValidationError(
                f"{file_path} case `{case_id}`: `must_not_route_to` must not contain `expected_skill`"
            )

    tags = require_string_list("tags", case.get("tags"), file_path, case_id)
    if len(set(tags)) != len(tags):
        raise CaseValidationError(f"{file_path} case `{case_id}`: `tags` contains duplicates")


def validate_suite(
    file_path: Path,
    skill_names: set[str],
    seen_ids: set[str],
    seen_queries: set[str],
    counts_by_skill: dict[str, int],
) -> None:
    payload = load_json(file_path)
    if not isinstance(payload, dict):
        raise CaseValidationError(f"{file_path}: top-level JSON must be an object")

    suite_skill = require_string("suite", payload.get("suite"), file_path)
    if suite_skill not in skill_names:
        raise CaseValidationError(f"{file_path}: unknown suite `{suite_skill}`")
    if file_path.stem != suite_skill:
        raise CaseValidationError(f"{file_path}: file name must match suite `{suite_skill}`")

    require_string("description", payload.get("description"), file_path)

    cases = payload.get("cases")
    if not isinstance(cases, list) or not cases:
        raise CaseValidationError(f"{file_path}: field `cases` must be a non-empty list")

    for case in cases:
        validate_case(case, file_path, suite_skill, skill_names, seen_ids, seen_queries)

    counts_by_skill[suite_skill] += len(cases)


def main() -> int:
    skill_names = set(iter_skill_names())
    counts_by_skill = {skill_name: 0 for skill_name in skill_names}
    seen_ids: set[str] = set()
    seen_queries: set[str] = set()

    if not EVAL_CASES_ROOT.exists():
        print(f"[ERROR] missing eval cases directory: {EVAL_CASES_ROOT}", file=sys.stderr)
        return 1

    case_files = sorted(EVAL_CASES_ROOT.glob("*.json"))
    if not case_files:
        print(f"[ERROR] no eval case files found in {EVAL_CASES_ROOT}", file=sys.stderr)
        return 1

    try:
        for case_file in case_files:
            validate_suite(case_file, skill_names, seen_ids, seen_queries, counts_by_skill)
    except (CaseValidationError, json.JSONDecodeError) as exc:
        print(f"[ERROR] {exc}", file=sys.stderr)
        return 1

    for skill_name, minimum in MIN_CASES_BY_SKILL.items():
        actual = counts_by_skill.get(skill_name, 0)
        if actual < minimum:
            print(
                f"[ERROR] insufficient eval coverage for `{skill_name}`: expected >= {minimum}, got {actual}",
                file=sys.stderr,
            )
            return 1

    total_cases = sum(counts_by_skill.values())
    print(f"[OK] validated {total_cases} eval cases across {len(case_files)} suites")
    return 0


if __name__ == "__main__":
    sys.exit(main())

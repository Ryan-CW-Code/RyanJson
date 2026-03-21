#!/usr/bin/env python3
"""Validate RyanJson skills metadata, routes, and markdown references."""

from __future__ import annotations

import re
import sys
from pathlib import Path
from typing import Optional


REPO_ROOT = Path(__file__).resolve().parents[2]
SKILLS_ROOT = REPO_ROOT / "skills"
SKILL_NAME_RE = re.compile(r"^[a-z0-9-]{1,64}$")
HEADING_RE = re.compile(r"^(#{1,6})\s+(.+?)\s*$")
FRONTMATTER_RE = re.compile(r"^---\n(.*?)\n---\n?", re.DOTALL)
FRONTMATTER_FIELD_RE = re.compile(r"^([A-Za-z0-9_-]+):\s*(.*)$")
OPENAI_FIELD_RE = re.compile(r"^[ \t]*([A-Za-z0-9_]+):[ \t]*(.+)$")
GEMINI_RE = re.compile(r"\bgemini\b", re.IGNORECASE)
ROOT_PREFIXES = (
    "skills/",
    "RyanJson/",
    "test/",
    "scripts/",
    ".agent/",
    ".github/",
    "example/",
    "reports/",
    "localLogs/",
    "coverage/",
    "build/",
    "xmake/",
    "run_local_",
)
LOCAL_PREFIXES = (
    "./",
    "../",
    "references/",
    "agents/",
)
ROOT_FILES = {
    "AGENTS.md",
    "README.md",
    "Makefile",
    "SConscript",
    "xmake.lua",
}
REFERENCE_SUFFIXES = {
    ".md",
    ".yaml",
    ".yml",
    ".sh",
    ".py",
}


class ValidationError(RuntimeError):
    """Raised when validation fails."""


def load_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def parse_frontmatter(skill_file: Path) -> dict[str, str]:
    text = load_text(skill_file)
    match = FRONTMATTER_RE.match(text)
    if match is None:
        raise ValidationError(f"{skill_file}: missing YAML frontmatter")

    fields: dict[str, str] = {}
    for raw_line in match.group(1).splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        field_match = FRONTMATTER_FIELD_RE.match(line)
        if field_match is None:
            continue
        key = field_match.group(1)
        value = field_match.group(2).strip().strip("'\"")
        fields[key] = value
    return fields


def validate_frontmatter(skill_dir: Path) -> list[str]:
    warnings: list[str] = []
    skill_file = skill_dir / "SKILL.md"
    if not skill_file.exists():
        raise ValidationError(f"{skill_dir}: missing SKILL.md")

    fields = parse_frontmatter(skill_file)
    name = fields.get("name", "")
    description = fields.get("description", "")

    if not name:
        raise ValidationError(f"{skill_file}: missing frontmatter field 'name'")
    if not SKILL_NAME_RE.fullmatch(name):
        raise ValidationError(
            f"{skill_file}: invalid name '{name}' (expected lowercase letters/digits/hyphens, <= 64 chars)"
        )
    if "anthropic" in name or "claude" in name:
        raise ValidationError(f"{skill_file}: invalid reserved substring in name '{name}'")
    if skill_dir.name != name:
        raise ValidationError(f"{skill_file}: directory name '{skill_dir.name}' does not match skill name '{name}'")

    if not description:
        raise ValidationError(f"{skill_file}: missing frontmatter field 'description'")
    if len(description) > 1024:
        raise ValidationError(f"{skill_file}: description exceeds 1024 characters")
    if "<" in description or ">" in description:
        raise ValidationError(f"{skill_file}: description contains XML-like angle brackets")

    body_lines = load_text(skill_file).splitlines()
    if len(body_lines) > 500:
        warnings.append(f"{skill_file}: body is longer than recommended 500 lines")
    if len(description) > 200:
        warnings.append(f"{skill_file}: description exceeds recommended 200 characters for Claude.ai compatibility")

    return warnings


def validate_openai_yaml(skill_dir: Path) -> None:
    skill_file = skill_dir / "SKILL.md"
    skill_name = parse_frontmatter(skill_file)["name"]
    openai_file = skill_dir / "agents" / "openai.yaml"

    if not openai_file.exists():
        raise ValidationError(f"{skill_dir}: missing agents/openai.yaml")

    fields: dict[str, str] = {}
    for raw_line in load_text(openai_file).splitlines():
        match = OPENAI_FIELD_RE.match(raw_line)
        if match is None:
            continue
        key = match.group(1)
        value = match.group(2).strip().strip("'\"")
        fields[key] = value

    for key in ("display_name", "short_description", "default_prompt"):
        value = fields.get(key, "")
        if not value:
            raise ValidationError(f"{openai_file}: missing interface field '{key}'")

    if f"${skill_name}" not in fields["default_prompt"]:
        raise ValidationError(f"{openai_file}: default_prompt must reference ${skill_name}")


def should_check_path(ref: str) -> bool:
    if not ref or any(char in ref for char in "*?[]"):
        return False
    if "://" in ref:
        return False
    if ref in {"references/", "agents/"}:
        return False
    if ref in ROOT_FILES:
        return True
    if ref.startswith(ROOT_PREFIXES) or ref.startswith(LOCAL_PREFIXES):
        return True
    return Path(ref).suffix.lower() in REFERENCE_SUFFIXES


def iter_reference_candidates(code_span: str) -> list[str]:
    candidates: list[str] = []
    seen: set[str] = set()

    for raw_token in code_span.split():
        token = raw_token.strip("`'\"(),;:")
        if not should_check_path(token):
            continue
        if token in seen:
            continue
        seen.add(token)
        candidates.append(token)

    if candidates:
        return candidates

    token = code_span.strip("`'\"(),;:")
    if should_check_path(token):
        return [token]
    return []


def find_repo_matches(ref: str) -> list[Path]:
    normalized = ref.rstrip("/")
    if normalized.startswith("./"):
        normalized = normalized[2:]

    while normalized.startswith("../"):
        normalized = normalized[3:]

    if not normalized:
        return []

    matches: list[Path] = []

    if "/" in normalized:
        for path in REPO_ROOT.rglob(Path(normalized).name):
            rel = path.relative_to(REPO_ROOT).as_posix()
            if rel.endswith(normalized):
                matches.append(path)
        return sorted(set(matches))

    return sorted(set(REPO_ROOT.rglob(normalized)))


def resolve_reference(doc_path: Path, ref: str) -> Optional[Path]:
    if ref in ROOT_FILES or ref.startswith(ROOT_PREFIXES):
        target = (REPO_ROOT / ref).resolve()
        return target if target.exists() else None
    if ref.startswith(LOCAL_PREFIXES):
        target = (doc_path.parent / ref).resolve()
        return target if target.exists() else None
    if Path(ref).suffix.lower() in REFERENCE_SUFFIXES:
        target = (doc_path.parent / ref).resolve()
        if target.exists():
            return target

    matches = find_repo_matches(ref)
    if len(matches) == 1:
        return matches[0].resolve()
    return None


def validate_markdown_references(doc_path: Path) -> list[str]:
    errors: list[str] = []
    seen: set[tuple[int, str]] = set()

    for lineno, line in enumerate(load_text(doc_path).splitlines(), start=1):
        for ref in re.findall(r"`([^`]+)`", line):
            for candidate in iter_reference_candidates(ref):
                key = (lineno, candidate)
                if key in seen:
                    continue
                seen.add(key)

                target = resolve_reference(doc_path, candidate)
                if target is None and not find_repo_matches(candidate):
                    rel_doc = doc_path.relative_to(REPO_ROOT)
                    errors.append(f"{rel_doc}:{lineno}: missing markdown reference `{candidate}`")

    return errors


def iter_skill_dirs() -> list[Path]:
    return sorted(path for path in SKILLS_ROOT.iterdir() if path.is_dir() and (path / "SKILL.md").exists())


def iter_markdown_files() -> list[Path]:
    files = [REPO_ROOT / "AGENTS.md"]
    files.extend(sorted(SKILLS_ROOT.rglob("*.md")))
    return [path for path in files if path.exists()]


def validate_agents_routes() -> list[str]:
    errors: list[str] = []
    agents_doc = load_text(REPO_ROOT / "AGENTS.md")

    for skill_dir in iter_skill_dirs():
        route = f"skills/{skill_dir.name}/SKILL.md"
        if route not in agents_doc:
            errors.append(f"AGENTS.md: missing route for `{route}`")

    return errors


def validate_no_gemini_assets() -> list[str]:
    errors: list[str] = []
    forbidden_files = [
        REPO_ROOT / ".agent" / "rules" / "gemini.md",
    ]
    forbidden_files.extend(SKILLS_ROOT.rglob("agents/gemini.md"))
    forbidden_files.extend(SKILLS_ROOT.rglob("references/geminiCompat.md"))

    for path in forbidden_files:
        if path.exists():
            errors.append(f"{path.relative_to(REPO_ROOT)}: Gemini-specific asset is not allowed")

    scan_files = [REPO_ROOT / "AGENTS.md"]
    scan_files.extend(sorted(SKILLS_ROOT.rglob("*.md")))
    scan_files.extend(sorted((REPO_ROOT / ".agent").rglob("*.md")) if (REPO_ROOT / ".agent").exists() else [])

    for path in scan_files:
        text = load_text(path)
        if GEMINI_RE.search(text):
            errors.append(f"{path.relative_to(REPO_ROOT)}: Gemini-specific text is not allowed")

    return errors


def main() -> int:
    warnings: list[str] = []
    errors: list[str] = []

    if not SKILLS_ROOT.exists():
        print(f"[ERROR] missing skills directory: {SKILLS_ROOT}", file=sys.stderr)
        return 1

    for skill_dir in iter_skill_dirs():
        try:
            warnings.extend(validate_frontmatter(skill_dir))
            validate_openai_yaml(skill_dir)
        except ValidationError as exc:
            errors.append(str(exc))

    errors.extend(validate_no_gemini_assets())
    errors.extend(validate_agents_routes())

    for doc_path in iter_markdown_files():
        errors.extend(validate_markdown_references(doc_path))

    for warning in warnings:
        print(f"[WARN] {warning}")

    if errors:
        for error in errors:
            print(f"[ERROR] {error}", file=sys.stderr)
        return 1

    print(f"[OK] validated {len(iter_skill_dirs())} skills and markdown references")
    return 0


if __name__ == "__main__":
    sys.exit(main())

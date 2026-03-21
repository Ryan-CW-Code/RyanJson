#!/bin/bash
set -euo pipefail

# 本地一键 skills 同步与规范校验。
# 默认行为：
#   1) 同步各技能的 references/terminology.md 到统一占位模板
#   2) 使用 skill-creator 的 quick_validate.py 校验每个技能
#   3) 校验 agents/openai.yaml 关键字段与默认 prompt 的技能名引用
#
# 可选参数：
#   --sync-only      仅执行同步
#   --validate-only  仅执行校验

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=scripts/lib/common.sh
source "${scriptDir}/scripts/lib/common.sh"
repoRoot="$(ryanjson_repo_root_from_source "${BASH_SOURCE[0]}" 0)"
cd "${repoRoot}"

doSync=1
doValidate=1
validator="/root/.codex/skills/.system/skill-creator/scripts/quick_validate.py"

declare -a skillDirs=()

parse_args() {
	# 解析参数（仅支持同步/校验开关）
	while (($# > 0)); do
		case "$1" in
		--sync-only)
			doValidate=0
			;;
		--validate-only)
			doSync=0
			;;
		-h | --help)
			echo "用法: bash ./run_local_skills.sh [--sync-only|--validate-only]"
			exit 0
			;;
		*)
			ryanjson_log_error "未知参数: $1"
			echo "用法: bash ./run_local_skills.sh [--sync-only|--validate-only]"
			exit 2
			;;
		esac
		shift
	done
}

load_skills() {
	# 收集 skills 目录（排除 shared）
	mapfile -t skillDirs < <(find skills -mindepth 1 -maxdepth 1 -type d ! -name shared | sort)
	if [[ ${#skillDirs[@]} -eq 0 ]]; then
		ryanjson_log_info "未发现可处理的技能目录（skills/*，排除 skills/shared）"
		exit 0
	fi
}

sync_terms() {
	ryanjson_log_phase "同步术语占位文档..."
	for skillDir in "${skillDirs[@]}"; do
		termFile="${skillDir}/references/terminology.md"
		mkdir -p "$(dirname "${termFile}")"
		if [[ ! -f "${termFile}" ]]; then
			cat > "${termFile}" <<'INNER_EOF'
# 术语字典

- 统一术语定义复用共享文档：`../../shared/terminology.md`。
- 如出现本技能专属术语，可在本文件追加扩展，不覆盖共享定义。
INNER_EOF
			ryanjson_log_info "synced ${termFile} (created)"
		elif grep -Fq '../../shared/terminology.md' "${termFile}"; then
			ryanjson_log_info "synced ${termFile} (already linked)"
		else
			local tmpRoot="${repoRoot}/localLogs/_tmp"
			mkdir -p "${tmpRoot}"
			tmpFile="$(mktemp "${tmpRoot}/terminology.XXXXXX")"
			cat > "${tmpFile}" <<'INNER_EOF'
# 术语字典

- 统一术语定义复用共享文档：`../../shared/terminology.md`。
- 如出现本技能专属术语，可在本文件追加扩展，不覆盖共享定义。

INNER_EOF
			cat "${termFile}" >> "${tmpFile}"
			mv "${tmpFile}" "${termFile}"
			ryanjson_log_info "synced ${termFile} (prefixed)"
		fi
	done
}

validate_skills() {
	ryanjson_log_phase "校验技能结构与 agents 元数据..."
	for skillDir in "${skillDirs[@]}"; do
		skillFile="${skillDir}/SKILL.md"
		openaiFile="${skillDir}/agents/openai.yaml"

		python3 "${validator}" "${skillDir}" >/dev/null

		if [[ ! -f "${openaiFile}" ]]; then
			ryanjson_log_error "缺少 agents/openai.yaml: ${openaiFile}"
			exit 1
		fi

		if ! grep -Eq '^[[:space:]]*display_name:' "${openaiFile}"; then
			ryanjson_log_error "缺少 interface.display_name: ${openaiFile}"
			exit 1
		fi
		if ! grep -Eq '^[[:space:]]*short_description:' "${openaiFile}"; then
			ryanjson_log_error "缺少 interface.short_description: ${openaiFile}"
			exit 1
		fi
		if ! grep -Eq '^[[:space:]]*default_prompt:' "${openaiFile}"; then
			ryanjson_log_error "缺少 interface.default_prompt: ${openaiFile}"
			exit 1
		fi

		skillName="$(awk '
			BEGIN { inFm=0 }
			/^---[[:space:]]*$/ { if (inFm==0) { inFm=1; next } else { exit } }
			inFm==1 && /^name:[[:space:]]*/ {
				sub(/^name:[[:space:]]*/, "", $0)
				print $0
				exit
			}
		' "${skillFile}")"

		if [[ -z "${skillName}" ]]; then
			ryanjson_log_error "未能从 ${skillFile} 读取 name"
			exit 1
		fi

		if ! grep -Fq "\$${skillName}" "${openaiFile}"; then
			ryanjson_log_error "default_prompt 未引用 \$${skillName}: ${openaiFile}"
			exit 1
		fi

		ryanjson_log_info "valid ${skillDir}"
	done
}

main() {
	parse_args "$@"

	if [[ ! -f "${validator}" ]]; then
		ryanjson_log_error "未找到技能校验脚本: ${validator}"
		exit 1
	fi

	load_skills

	ryanjson_print_banner_begin "本地 Skills 任务启动"
	ryanjson_print_banner_kv "sync" "${doSync}"
	ryanjson_print_banner_kv "validate" "${doValidate}"
	ryanjson_print_banner_kv "skills" "${#skillDirs[@]}"
	ryanjson_print_banner_end

	if [[ ${doSync} -eq 1 ]]; then
		sync_terms
	fi

	if [[ ${doValidate} -eq 1 ]]; then
		validate_skills
	fi

	ryanjson_print_banner_begin "本地 Skills 任务完成"
	ryanjson_print_banner_end
}

main "$@"

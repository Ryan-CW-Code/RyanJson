#!/usr/bin/env bash

ryanjson_abs_dir() {
  local inputPath="$1"
  (cd "$(dirname "${inputPath}")" && pwd)
}

ryanjson_repo_root_from_source() {
  local sourcePath="$1"
  local levelsUp="${2:-0}"
  local currentDir=""

  currentDir="$(ryanjson_abs_dir "${sourcePath}")"
  while (( levelsUp > 0 )); do
    currentDir="$(cd "${currentDir}/.." && pwd)"
    levelsUp=$((levelsUp - 1))
  done

  printf '%s\n' "${currentDir}"
}

ryanjson_print_banner_begin() {
  local title="$1"
  echo "===================================================="
  echo "${title}"
}

ryanjson_print_banner_kv() {
  local key="$1"
  local value="$2"
  echo "  - ${key}=${value}"
}

ryanjson_print_banner_end() {
  echo "===================================================="
}

ryanjson_require_cmd() {
  local cmdName="$1"
  local hint="${2:-}"

  if ! command -v "${cmdName}" >/dev/null 2>&1; then
    echo "[错误] 缺少命令: ${cmdName}"
    if [[ -n "${hint}" ]]; then
      echo "[提示] ${hint}"
    fi
    return 1
  fi
}

ryanjson_prepare_clean_dir() {
  local dirPath="$1"
  rm -rf "${dirPath}"
  mkdir -p "${dirPath}"
}

ryanjson_run_xmake_config() {
  local forceClean="$1"
  local caseName="$2"

  if [[ "${forceClean}" == "1" ]]; then
    echo "[阶段] 正在执行 xmake 配置（clean 模式）..."
    if ! xmake f -c; then
      echo "[错误] xmake 配置失败：${caseName}"
      return 1
    fi
  else
    echo "[阶段] 正在执行 xmake 配置（增量模式）..."
    if ! xmake f; then
      echo "[错误] xmake 配置失败：${caseName}"
      return 1
    fi
  fi
}

ryanjson_run_xmake_build() {
  local targetName="$1"
  local caseName="$2"

  echo "[阶段] 正在执行 xmake 构建（target=${targetName}）..."
  if ! xmake -b "${targetName}"; then
    echo "[错误] xmake 构建失败：${caseName}"
    return 1
  fi
}

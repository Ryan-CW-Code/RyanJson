#!/usr/bin/env bash

# 通用脚本工具函数（供 run_local_* 使用）

ryanjson_init_utf8_locale() {
  # Force UTF-8 locale for script-generated files/logs when available.
  local localeValue=""

  if command -v locale >/dev/null 2>&1; then
    if locale -a 2>/dev/null | grep -qx "C.UTF-8"; then
      localeValue="C.UTF-8"
    elif locale -a 2>/dev/null | grep -qx "en_US.UTF-8"; then
      localeValue="en_US.UTF-8"
    fi
  fi

  if [[ -n "${localeValue}" ]]; then
    export LC_ALL="${localeValue}"
    export LANG="${localeValue}"
  fi
}

ryanjson_init_utf8_locale

ryanjson_init_utf8_log() {
  # Initialize a UTF-8 log file (no BOM).
  local path="$1"
  mkdir -p "$(dirname "${path}")"
  : > "${path}"
}

ryanjson_abs_dir() {
  # 返回文件/目录的绝对路径（仅取目录部分）
  local inputPath="$1"
  (cd "$(dirname "${inputPath}")" && pwd)
}

ryanjson_repo_root_from_source() {
  # 从脚本路径向上回溯，拿到仓库根目录
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
  # 统一 banner 输出（开始）
  local title="$1"
  echo "===================================================="
  echo "${title}"
}

ryanjson_print_banner_kv() {
  # 统一 banner 输出（KV）
  local key="$1"
  local value="$2"
  echo "  - ${key}=${value}"
}

ryanjson_print_banner_kv_optional() {
  # 仅在值非空时输出 banner KV
  local key="$1"
  local value="$2"
  if [[ -n "${value}" ]]; then
    ryanjson_print_banner_kv "${key}" "${value}"
  fi
}

ryanjson_print_banner_end() {
  # 统一 banner 输出（结束）
  echo "===================================================="
}

ryanjson_log_phase() {
  # 阶段提示
  echo "[阶段] $*"
}

ryanjson_log_info() {
  # 普通信息
  echo "[信息] $*"
}

ryanjson_log_warn() {
  # 警告提示
  echo "[警告] $*"
}

ryanjson_log_error() {
  # 错误提示
  echo "[错误] $*"
}

ryanjson_log_done() {
  # 完成提示
  echo "[完成] $*"
}

ryanjson_is_nonneg_int() {
  # 非负整数判断
  [[ "$1" =~ ^[0-9]+$ ]]
}

ryanjson_is_positive_int() {
  # 正整数判断
  [[ "$1" =~ ^[0-9]+$ ]] && (( $1 > 0 ))
}

ryanjson_is_01() {
  # 0/1 开关判断
  [[ "$1" == "0" || "$1" == "1" ]]
}

ryanjson_require_01() {
  # 校验 0/1 开关
  local name="$1"
  local value="$2"
  if ! ryanjson_is_01 "${value}"; then
    ryanjson_log_error "${name} 仅支持 0/1，当前值：${value}"
    return 1
  fi
}

ryanjson_require_pos_int() {
  # 校验正整数（可选附加单位）
  local name="$1"
  local value="$2"
  local unit="${3:-}"
  if ! ryanjson_is_positive_int "${value}"; then
    if [[ -n "${unit}" ]]; then
      ryanjson_log_error "${name} 仅支持正整数（${unit}），当前值：${value}"
    else
      ryanjson_log_error "${name} 仅支持正整数，当前值：${value}"
    fi
    return 1
  fi
}

ryanjson_require_nonneg_int() {
  # 校验非负整数
  local name="$1"
  local value="$2"
  if ! ryanjson_is_nonneg_int "${value}"; then
    ryanjson_log_error "${name} 仅支持非负整数，当前值：${value}"
    return 1
  fi
}

ryanjson_require_pos_int_optional() {
  # 可选正整数校验（空值跳过）
  local name="$1"
  local value="$2"
  local unit="${3:-}"
  if [[ -n "${value}" ]]; then
    ryanjson_require_pos_int "${name}" "${value}" "${unit}"
    return $?
  fi
  return 0
}

ryanjson_require_nonneg_int_optional() {
  # 可选非负整数校验（空值跳过）
  local name="$1"
  local value="$2"
  if [[ -n "${value}" ]]; then
    ryanjson_require_nonneg_int "${name}" "${value}"
    return $?
  fi
  return 0
}

ryanjson_require_cmds() {
  # 批量检查命令依赖
  local hint="${1:-}"
  shift || true
  local missing=0

  for cmdName in "$@"; do
    if ! command -v "${cmdName}" >/dev/null 2>&1; then
      ryanjson_log_error "缺少命令: ${cmdName}"
      missing=1
    fi
  done

  if [[ "${missing}" -ne 0 && -n "${hint}" ]]; then
    ryanjson_log_info "提示: ${hint}"
  fi

  return "${missing}"
}

ryanjson_require_cmd() {
  # 单命令检查（复用批量版本）
  local cmdName="$1"
  local hint="${2:-}"

  ryanjson_require_cmds "${hint}" "${cmdName}"
}

ryanjson_prepare_clean_dir() {
  # 清理并重新创建目录
  local dirPath="$1"
  rm -rf "${dirPath}"
  mkdir -p "${dirPath}"
}

ryanjson_run_xmake_config() {
  # 运行 xmake 配置（支持 clean/增量）
  local forceClean="$1"
  local caseName="$2"

  if [[ "${forceClean}" == "1" ]]; then
    ryanjson_log_phase "正在执行 xmake 配置（clean 模式）..."
    if ! xmake f -c; then
      ryanjson_log_error "xmake 配置失败：${caseName}"
      return 1
    fi
  else
    ryanjson_log_phase "正在执行 xmake 配置（增量模式）..."
    if ! xmake f; then
      ryanjson_log_error "xmake 配置失败：${caseName}"
      return 1
    fi
  fi
}

ryanjson_run_xmake_build() {
  # 运行 xmake 构建
  local targetName="$1"
  local caseName="$2"

  ryanjson_log_phase "正在执行 xmake 构建（target=${targetName}）..."
  if ! xmake -b "${targetName}"; then
    ryanjson_log_error "xmake 构建失败：${caseName}"
    return 1
  fi
}

ryanjson_export_semantic_macros() {
  # 统一导出语义宏（strict/addAtHead/scientific）
  local strictKey="$1"
  local addAtHead="$2"
  local scientific="$3"

  export RYANJSON_STRICT_OBJECT_KEY_CHECK="${strictKey}"
  export RYANJSON_DEFAULT_ADD_AT_HEAD="${addAtHead}"
  export RYANJSON_SNPRINTF_SUPPORT_SCIENTIFIC="${scientific}"
}

ryanjson_print_semantic_kv() {
  # 统一输出语义宏 KV（配合 banner 使用）
  local strictKey="$1"
  local addAtHead="$2"
  local scientific="$3"

  ryanjson_print_banner_kv "RyanJsonStrictObjectKeyCheck" "${strictKey}"
  ryanjson_print_banner_kv "RyanJsonDefaultAddAtHead" "${addAtHead}"
  ryanjson_print_banner_kv "RyanJsonSnprintfSupportScientific" "${scientific}"
}

ryanjson_semantic_log_error() {
  # 语义矩阵输出错误提示
  ryanjson_log_error "$@"
}

ryanjson_emit_semantic_cases() {
  # 根据模式输出三元组：strictKey addAtHead scientific
  local mode="$1"
  local modeName="${2:-MODE}"

  case "${mode}" in
    quick)
      printf '%s\n' 'false true true' 'true false true'
      ;;
    nightly)
      local strictKey=""
      local addAtHead=""
      for strictKey in false true; do
        for addAtHead in false true; do
          printf '%s %s true\n' "${strictKey}" "${addAtHead}"
        done
      done
      ;;
    full)
      local strictKey=""
      local addAtHead=""
      local scientific=""
      for strictKey in false true; do
        for addAtHead in false true; do
          for scientific in false true; do
            printf '%s %s %s\n' "${strictKey}" "${addAtHead}" "${scientific}"
          done
        done
      done
      ;;
    *)
      ryanjson_semantic_log_error "${modeName} 仅支持 quick/nightly/full，当前值：${mode}"
      return 1
      ;;
  esac
}

ryanjson_semantic_case_name() {
  # 生成可读的用例名称
  local strictKey="$1"
  local addAtHead="$2"
  local scientific="$3"
  printf 'strict_%s__head_%s__sci_%s\n' "${strictKey}" "${addAtHead}" "${scientific}"
}

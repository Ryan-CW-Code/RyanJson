#!/usr/bin/env bash

ryanjson_emit_semantic_cases() {
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
      echo "[错误] ${modeName} 仅支持 quick/nightly/full，当前值：${mode}" >&2
      return 1
      ;;
  esac
}

ryanjson_semantic_case_name() {
  local strictKey="$1"
  local addAtHead="$2"
  local scientific="$3"
  printf 'strict_%s__head_%s__sci_%s\n' "${strictKey}" "${addAtHead}" "${scientific}"
}

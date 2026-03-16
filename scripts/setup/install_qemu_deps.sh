#!/usr/bin/env bash
set -euo pipefail

# 安装 RyanJsonQemu 所需的 ARM 交叉工具链与 QEMU。
# 支持的包管理器：apt / dnf / yum / pacman / brew。

scriptDir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=../lib/common.sh
source "${scriptDir}/../lib/common.sh"

SCRIPT_NAME="$(basename "$0")"
NO_UPDATE="0"

usage() {
  cat <<USAGE
用法: ${SCRIPT_NAME} [--no-update]

选项:
  --no-update   跳过包索引刷新步骤（apt/dnf/yum/pacman）。
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --no-update)
      NO_UPDATE="1"
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      ryanjson_log_error "未知参数: $1"
      usage
      exit 1
      ;;
  esac
done

if [[ $(id -u) -eq 0 ]]; then
  SUDO=""
else
  if command -v sudo >/dev/null 2>&1; then
    SUDO="sudo"
  else
    ryanjson_log_error "非 root 环境需要 sudo 权限。"
    exit 1
  fi
fi

log() {
  # 安装日志输出
  ryanjson_log_info "install_qemu_deps: $*"
}

have_cmd() {
  # 判断命令是否存在
  command -v "$1" >/dev/null 2>&1
}

install_with_apt() {
  # Ubuntu/Debian 系列安装
  local -a pkgs=(
    gcc-arm-none-eabi
    binutils-arm-none-eabi
    qemu-system-arm
    qemu-utils
  )

  if [[ "${NO_UPDATE}" != "1" ]]; then
    log "apt-get update"
    ${SUDO} apt-get update
  fi

  log "apt-get install: ${pkgs[*]}"
  DEBIAN_FRONTEND=noninteractive ${SUDO} apt-get install -y "${pkgs[@]}"
}

install_with_dnf() {
  # Fedora/CentOS Stream 系列安装
  local -a pkgs=(
    arm-none-eabi-gcc-cs
    arm-none-eabi-binutils-cs
    qemu-system-arm
  )

  if [[ "${NO_UPDATE}" != "1" ]]; then
    log "dnf makecache"
    ${SUDO} dnf -y makecache
  fi

  log "dnf install: ${pkgs[*]}"
  ${SUDO} dnf install -y "${pkgs[@]}"
}

install_with_yum() {
  # CentOS/RHEL 系列安装
  local -a pkgs=(
    arm-none-eabi-gcc-cs
    arm-none-eabi-binutils-cs
    qemu-system-arm
  )

  if [[ "${NO_UPDATE}" != "1" ]]; then
    log "yum makecache"
    ${SUDO} yum -y makecache
  fi

  log "yum install: ${pkgs[*]}"
  ${SUDO} yum install -y "${pkgs[@]}"
}

install_with_pacman() {
  # Arch 系列安装
  local -a pkgs=(
    arm-none-eabi-gcc
    arm-none-eabi-binutils
    qemu-system-arm
  )

  if [[ "${NO_UPDATE}" != "1" ]]; then
    log "pacman -Sy"
    ${SUDO} pacman -Sy --noconfirm
  fi

  log "pacman install: ${pkgs[*]}"
  ${SUDO} pacman -S --noconfirm --needed "${pkgs[@]}"
}

install_with_brew() {
  # macOS Homebrew 安装
  local -a pkgs=(
    arm-none-eabi-gcc
    qemu
  )

  log "brew install: ${pkgs[*]}"
  brew install "${pkgs[@]}"
}

install_deps() {
  # 选择可用的包管理器
  if have_cmd apt-get; then
    install_with_apt
    return
  fi
  if have_cmd dnf; then
    install_with_dnf
    return
  fi
  if have_cmd yum; then
    install_with_yum
    return
  fi
  if have_cmd pacman; then
    install_with_pacman
    return
  fi
  if have_cmd brew; then
    install_with_brew
    return
  fi

  ryanjson_log_error "未识别到受支持的包管理器，请手动安装："
  echo "  - arm-none-eabi-gcc"
  echo "  - arm-none-eabi-objcopy (binutils)"
  echo "  - qemu-system-arm"
  exit 1
}

verify() {
  # 安装后检查必需命令
  local missing=0
  for cmd in arm-none-eabi-gcc arm-none-eabi-objcopy qemu-system-arm; do
    if have_cmd "${cmd}"; then
      log "found ${cmd}: $(command -v "${cmd}")"
    else
      ryanjson_log_error "安装后仍缺少命令: ${cmd}"
      missing=1
    fi
  done

  if [[ "${missing}" -ne 0 ]]; then
    exit 1
  fi

  arm-none-eabi-gcc --version | head -n 1
  arm-none-eabi-objcopy --version | head -n 1
  qemu-system-arm --version | head -n 1

  log "依赖准备完成。"
}

install_deps
verify

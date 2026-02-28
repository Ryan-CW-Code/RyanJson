#!/usr/bin/env bash
set -euo pipefail

# Install ARM bare-metal toolchain + QEMU needed by RyanJsonQemu.
# Supported package managers: apt, dnf, yum, pacman, brew.

SCRIPT_NAME="$(basename "$0")"
NO_UPDATE="0"

usage() {
  cat <<USAGE
Usage: ${SCRIPT_NAME} [--no-update]

Options:
  --no-update   Skip package index refresh step (apt/dnf/yum/pacman).
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
      echo "[ERROR] Unknown argument: $1"
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
    echo "[ERROR] sudo is required when not running as root."
    exit 1
  fi
fi

log() {
  echo "[install_qemu_deps] $*"
}

have_cmd() {
  command -v "$1" >/dev/null 2>&1
}

install_with_apt() {
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
  local -a pkgs=(
    arm-none-eabi-gcc
    qemu
  )

  log "brew install: ${pkgs[*]}"
  brew install "${pkgs[@]}"
}

install_deps() {
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

  echo "[ERROR] Unsupported package manager. Please install manually:"
  echo "  - arm-none-eabi-gcc"
  echo "  - arm-none-eabi-objcopy (binutils)"
  echo "  - qemu-system-arm"
  exit 1
}

verify() {
  local missing=0
  for cmd in arm-none-eabi-gcc arm-none-eabi-objcopy qemu-system-arm; do
    if have_cmd "${cmd}"; then
      log "found ${cmd}: $(command -v "${cmd}")"
    else
      echo "[ERROR] missing command after install: ${cmd}"
      missing=1
    fi
  done

  if [[ "${missing}" -ne 0 ]]; then
    exit 1
  fi

  arm-none-eabi-gcc --version | head -n 1
  arm-none-eabi-objcopy --version | head -n 1
  qemu-system-arm --version | head -n 1

  log "Dependencies are ready."
}

install_deps
verify

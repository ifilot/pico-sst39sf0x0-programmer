#!/usr/bin/env bash
set -euo pipefail

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------
APP_NAME="pico-sst39sf0x0-programmer"
BUILD_DIR="${BUILD_DIR:-build-windows}"
DIST_DIR="${DIST_DIR:-dist}"
INSTALLER_DIR="${INSTALLER_DIR:-installer}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

echo "==> Packaging ${APP_NAME} (Windows / MinGW / Qt6)"

# ------------------------------------------------------------
# Validate existing build
# ------------------------------------------------------------
if [[ ! -f "${BUILD_DIR}/${APP_NAME}.exe" ]]; then
  echo "[ERROR] Expected built executable at ${BUILD_DIR}/${APP_NAME}.exe"
  echo "[ERROR] Run the configure/build/test steps before packaging."
  exit 1
fi

rm -rf "${DIST_DIR}"
mkdir -p "${BUILD_DIR}" "${DIST_DIR}"

# ------------------------------------------------------------
# Copy executable
# ------------------------------------------------------------
echo "==> Copying executable"
cp "${BUILD_DIR}/${APP_NAME}.exe" "${DIST_DIR}/"

# ------------------------------------------------------------
# Deploy Qt runtime
# ------------------------------------------------------------
echo "==> Running windeployqt"
windeployqt6 \
  --release \
  --no-opengl-sw \
  --no-translations \
  --no-compiler-runtime \
  --openssl-root "$MINGW_PREFIX" \
  "${DIST_DIR}/${APP_NAME}.exe"

# ------------------------------------------------------------
# Optional: strip binary (smaller installer)
# ------------------------------------------------------------
strip "${DIST_DIR}/${APP_NAME}.exe" || true

ldd "${DIST_DIR}/${APP_NAME}.exe" | awk '{print $3}' | while read -r path; do
  case "$path" in
    /mingw64/bin/*.dll)
      dll="$(basename "$path")"
      echo "  + $dll"
      cp -n "$path" "${DIST_DIR}/"
      ;;
  esac
done

# Qt loads OpenSSL dynamically, so neither windeployqt nor ldd necessarily
# sees these DLLs in the normal import table. Deploy both parts of the matching
# MSYS2 OpenSSL runtime explicitly.
echo "==> Deploying OpenSSL runtime"
shopt -s nullglob
ssl_dlls=("$MINGW_PREFIX"/bin/libssl-*.dll)
crypto_dlls=("$MINGW_PREFIX"/bin/libcrypto-*.dll)
shopt -u nullglob

if [[ ${#ssl_dlls[@]} -eq 0 || ${#crypto_dlls[@]} -eq 0 ]]; then
  echo "[ERROR] Could not find the OpenSSL runtime DLLs in $MINGW_PREFIX/bin"
  exit 1
fi

cp "${ssl_dlls[@]}" "${crypto_dlls[@]}" "${DIST_DIR}/"

# Exercise TLS from the actual deployment directory with the MinGW toolchain
# removed from PATH. This catches missing or incompatible runtime DLLs before
# an installer can be published.
echo "==> Testing deployed TLS runtime"
SMOKE_TEST="deployment_ssl_smoke_test.exe"
if [[ ! -f "${BUILD_DIR}/${SMOKE_TEST}" ]]; then
  echo "[ERROR] Expected deployment smoke test at ${BUILD_DIR}/${SMOKE_TEST}"
  exit 1
fi
cp "${BUILD_DIR}/${SMOKE_TEST}" "${DIST_DIR}/"
if ! PATH="/c/Windows/System32:/c/Windows" "${DIST_DIR}/${SMOKE_TEST}"; then
  rm -f "${DIST_DIR}/${SMOKE_TEST}"
  echo "[ERROR] The packaged application cannot initialize TLS without the build environment."
  exit 1
fi
rm -f "${DIST_DIR}/${SMOKE_TEST}"

# ------------------------------------------------------------
# Build NSIS installer
# ------------------------------------------------------------
echo "==> Building NSIS installer"

# Read the application version and reject a mismatched release tag.
VERSION="$(sed -nE 's/^#define PROGRAM_VERSION "([0-9]+\.[0-9]+\.[0-9]+)"$/\1/p' gui/src/config.h)"
if [[ -z "$VERSION" ]]; then
  echo "[ERROR] Could not read PROGRAM_VERSION from gui/src/config.h"
  exit 1
fi

if git describe --tags --exact-match >/dev/null 2>&1; then
  TAG_VERSION="$(git describe --tags --exact-match | sed 's/^v//')"
  if [[ "$TAG_VERSION" != "$VERSION" ]]; then
    echo "[ERROR] Release tag v${TAG_VERSION} does not match PROGRAM_VERSION ${VERSION}"
    exit 1
  fi
fi

makensis \
  -DAPP_NAME="${APP_NAME}" \
  -DAPP_VERSION="${VERSION}" \
  "installer.nsi"

echo "==> Done"

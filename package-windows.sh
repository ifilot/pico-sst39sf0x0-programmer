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

echo "==> Packaging ${APP_NAME} (Windows / MinGW / Qt5)"

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

# Provide qmake.exe for windeployqt (MSYS2 requirement)
QMAKE_QT5="$MINGW_PREFIX/bin/qmake-qt5.exe"
QMAKE_SHIM="$MINGW_PREFIX/bin/qmake.exe"
if [[ ! -x "$QMAKE_SHIM" ]]; then
  echo "[INFO] Creating qmake.exe shim"
  ln -s qmake-qt5.exe "$QMAKE_SHIM" 2>/dev/null || cp "$QMAKE_QT5" "$QMAKE_SHIM"
fi

# ------------------------------------------------------------
# Deploy Qt runtime
# ------------------------------------------------------------
echo "==> Running windeployqt"
windeployqt-qt5 \
  --release \
  --no-angle \
  --no-opengl-sw \
  --no-translations \
  --no-compiler-runtime \
  "${DIST_DIR}/${APP_NAME}.exe"

# ------------------------------------------------------------
# Optional: strip binary (smaller installer)
# ------------------------------------------------------------
strip "${DIST_DIR}/${APP_NAME}.exe" || true

ldd "${DIST_DIR}/${APP_NAME}" | awk '{print $3}' | while read -r path; do
  case "$path" in
    /mingw64/bin/*.dll)
      dll="$(basename "$path")"
      echo "  + $dll"
      cp -n "$path" "${DIST_DIR}/"
      ;;
  esac
done

# ------------------------------------------------------------
# Build NSIS installer
# ------------------------------------------------------------
echo "==> Building NSIS installer"

# Detect version from git tag (if present)
VERSION="dev"
if git describe --tags --exact-match >/dev/null 2>&1; then
  VERSION="$(git describe --tags --exact-match | sed 's/^v//')"
fi

makensis \
  -DAPP_NAME="${APP_NAME}" \
  -DAPP_VERSION="${VERSION}" \
  "installer.nsi"

echo "==> Done"

#!/bin/bash
set -e

export PATH=/opt/xm_toolchain/arm-xm-linux/usr/bin:$PATH

PROJ_ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD_DIR="$PROJ_ROOT/build"

if [ "$1" == "clean" ]; then
    rm -rf "$BUILD_DIR"
    echo "clean done"
    exit 0
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake .. -D_WIFI_TYPE=2

make -j$(nproc)

echo ""
echo "==================== BUILD OK ===================="
ls -lh "$BUILD_DIR/wireless_box_receive"
file   "$BUILD_DIR/wireless_box_receive"
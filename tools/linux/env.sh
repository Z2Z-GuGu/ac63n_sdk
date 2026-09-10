#!/usr/bin/env bash

# 定位脚本真实路径（bash 用 BASH_SOURCE，zsh 回退到 $0）
_script="${BASH_SOURCE[0]:-$0}"
_SCRIPT_DIR="$(cd "$(dirname "$_script")" && pwd)"

# 1) SDK 根目录 = tools/linux 的上一级上一级
export AC632N_SDK_PATH="$(cd "$_SCRIPT_DIR/../.." && pwd)"

# 2) 工具链目录
export AC632N_TOOLCHAIN_PATH="$AC632N_SDK_PATH/tools/linux/toolchain"

# 3) 内核
export JL_CORE="q32s"

# 4) 芯片平台
export JL_CHIP_PLATFORM="bd19"

# 5) 工具链不存在则自动下载并解压
if [ ! -d "$AC632N_TOOLCHAIN_PATH/q32s" ]; then
    echo "[env.sh] 未找到工具链 $AC632N_TOOLCHAIN_PATH，开始自动下载..."
    _url="https://pkgman.jieliapp.com/s/linux-toolchain"
    _tmp="$(mktemp -d)"

    if ! curl -fL --retry 3 --retry-delay 2 -o "$_tmp/linux-toolchain.tar.xz" "$_url"; then
        rm -rf "$_tmp"
        echo "[env.sh] 错误: 工具链下载失败（$_url）" >&2
        return 1
    fi

    mkdir -p "$AC632N_TOOLCHAIN_PATH"
    if ! tar -xJf "$_tmp/linux-toolchain.tar.xz" -C "$_tmp" --strip-components=1; then
        rm -rf "$_tmp"
        echo "[env.sh] 错误: 工具链解压失败" >&2
        return 1
    fi

    # 删除压缩包
    rm -f "$_tmp/linux-toolchain.tar.xz"

    # 将解出的 common/pi32/pi32v2/q32s/README.md 移入 toolchain 目录
    for _item in "$_tmp"/*; do
        [ -e "$_item" ] && mv "$_item" "$AC632N_TOOLCHAIN_PATH"/
    done
    rm -rf "$_tmp"   # 删除临时目录

    echo "[env.sh] 工具链已就绪: $AC632N_TOOLCHAIN_PATH"
fi

echo "[env.sh] AC632N_SDK_PATH       = $AC632N_SDK_PATH"
echo "[env.sh] AC632N_TOOLCHAIN_PATH = $AC632N_TOOLCHAIN_PATH"
echo "[env.sh] JL_CORE               = $JL_CORE"
echo "[env.sh] JL_CHIP_PLATFORM      = $JL_CHIP_PLATFORM"

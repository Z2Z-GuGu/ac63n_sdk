#!/usr/bin/env bash

# ============================================================================
# init_toolchain.sh —— 一次性初始化脚本：
#   1) 检查 tools/linux/toolchain 是否有内容，没有则自动下载并解压；
#   2) 检查环境变量 AC63N_SDK_PATH 是否已存在，不存在则写入 ~/.bashrc
#      （指向本 SDK 目录）；
#   3) 将 AC63N_TOOLCHAIN_PATH=$AC63N_SDK_PATH/tools/linux/toolchain
#      写入 ~/.bashrc。
#
# 用法：
#   source tools/linux/init_toolchain.sh
# 或
#   bash tools/linux/init_toolchain.sh
#
# 写入 ~/.bashrc 后，新开终端即可直接使用这些环境变量。
# ============================================================================

# 定位脚本真实路径（bash 用 BASH_SOURCE，zsh 回退到 $0）
_script="${BASH_SOURCE[0]:-$0}"
_SCRIPT_DIR="$(cd "$(dirname "$_script")" && pwd)"

# 1) SDK 根目录 = tools/linux 的上一级上级
export AC63N_SDK_PATH="$(cd "$_SCRIPT_DIR/../.." && pwd)"

# 2) 工具链目录
export AC63N_TOOLCHAIN_PATH="$AC63N_SDK_PATH/tools/linux/toolchain"

# ---------------------------------------------------------------------------
# 步骤一：检测工具链目录是否有内容，没有则下载
# ---------------------------------------------------------------------------
if [ -z "$(ls -A "$AC63N_TOOLCHAIN_PATH" 2>/dev/null)" ]; then
    echo "[init_toolchain] 未找到工具链（$AC63N_TOOLCHAIN_PATH 为空或不存在），开始自动下载..."
    _url="https://pkgman.jieliapp.com/s/linux-toolchain"
    _tmp="$(mktemp -d)"

    if ! curl -fL --retry 3 --retry-delay 2 -o "$_tmp/linux-toolchain.tar.xz" "$_url"; then
        rm -rf "$_tmp"
        echo "[init_toolchain] 错误: 工具链下载失败（$_url）" >&2
        return 1 2>/dev/null || exit 1
    fi

    mkdir -p "$AC63N_TOOLCHAIN_PATH"
    if ! tar -xJf "$_tmp/linux-toolchain.tar.xz" -C "$_tmp" --strip-components=1; then
        rm -rf "$_tmp"
        echo "[init_toolchain] 错误: 工具链解压失败" >&2
        return 1 2>/dev/null || exit 1
    fi

    # 删除压缩包
    rm -f "$_tmp/linux-toolchain.tar.xz"

    # 将解出的 common/pi32/pi32v2/q32s/README.md 移入 toolchain 目录
    for _item in "$_tmp"/*; do
        [ -e "$_item" ] && mv "$_item" "$AC63N_TOOLCHAIN_PATH"/
    done
    rm -rf "$_tmp"   # 删除临时目录

    echo "[init_toolchain] 工具链已就绪: $AC63N_TOOLCHAIN_PATH"
else
    echo "[init_toolchain] 工具链已存在: $AC63N_TOOLCHAIN_PATH"
fi

# ---------------------------------------------------------------------------
# 步骤二：把 AC63N_SDK_PATH / AC63N_TOOLCHAIN_PATH 写入 ~/.bashrc（若缺失）
# ---------------------------------------------------------------------------
_BASHRC="$HOME/.bashrc"
touch "$_BASHRC"

# 判断一行是否已存在于 .bashrc（避免重复追加）
_has_line() {
    grep -qFx "$1" "$_BASHRC" 2>/dev/null
}

echo "[init_toolchain] 检查并写入环境变量到 ~/.bashrc ..."

# 写入 AC63N_SDK_PATH（若未包含该定义）
if ! _has_line "export AC63N_SDK_PATH=\"$AC63N_SDK_PATH\""; then
    printf 'export AC63N_SDK_PATH="%s"\n' "$AC63N_SDK_PATH" >> "$_BASHRC"
    echo "[init_toolchain] 已添加 AC63N_SDK_PATH=$AC63N_SDK_PATH"
else
    echo "[init_toolchain] AC63N_SDK_PATH 已存在于 ~/.bashrc，跳过"
fi

# 写入 AC63N_TOOLCHAIN_PATH（依赖 $AC63N_SDK_PATH）
if ! _has_line "export AC63N_TOOLCHAIN_PATH=\"\$AC63N_SDK_PATH/tools/linux/toolchain\"" \
   && ! grep -q 'AC63N_TOOLCHAIN_PATH=' "$_BASHRC"; then
    printf 'export AC63N_TOOLCHAIN_PATH="$AC63N_SDK_PATH/tools/linux/toolchain"\n' >> "$_BASHRC"
    echo "[init_toolchain] 已添加 AC63N_TOOLCHAIN_PATH=\$AC63N_SDK_PATH/tools/linux/toolchain"
else
    echo "[init_toolchain] AC63N_TOOLCHAIN_PATH 已存在于 ~/.bashrc，跳过"
fi

# ---------------------------------------------------------------------------
# 完成
# ---------------------------------------------------------------------------
echo "[init_toolchain] 完成。"
echo "[init_toolchain] 已在当前 shell 导出："
echo "[init_toolchain]   AC63N_SDK_PATH       = $AC63N_SDK_PATH"
echo "[init_toolchain]   AC63N_TOOLCHAIN_PATH = $AC63N_TOOLCHAIN_PATH"
echo "[init_toolchain] 新开终端后 ~/.bashrc 中的配置会自动生效。"
#!/usr/bin/env bash

# ============================================================================
# init_toolchain.sh —— 一次性初始化脚本：
#   1) 检查 tools/linux/toolchain 是否有内容，没有则自动下载并解压；
#   2) 检查 tools/linux 下是否有 jlisp 可执行文件，没有则从 GitHub
#      （https://github.com/Z2Z-GuGu/jlisp）下载最新 release 的 jlisp-*-linux-amd64
#      并拷贝到 tools/linux 下；
#   3) 检查环境变量 AC63N_SDK_PATH 是否已存在，不存在则写入 ~/.bashrc
#      （指向本 SDK 目录）；
#   4) 将 AC63N_TOOLS_PATH=$AC63N_SDK_PATH/tools/linux
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

# 2) tools 目录（含 toolchain / jlisp 等）
export AC63N_TOOLS_PATH="$AC63N_SDK_PATH/tools/linux"

# 工具链目录 = $AC63N_TOOLS_PATH/toolchain
_TOOLCHAIN_PATH="$AC63N_TOOLS_PATH/toolchain"

# ---------------------------------------------------------------------------
# 步骤一：检测工具链目录是否有内容，没有则下载
# ---------------------------------------------------------------------------
if [ -z "$(ls -A "$_TOOLCHAIN_PATH" 2>/dev/null)" ]; then
    echo "[init_toolchain] 未找到工具链（$_TOOLCHAIN_PATH 为空或不存在），开始自动下载..."
    _url="https://pkgman.jieliapp.com/s/linux-toolchain"
    _tmp="$(mktemp -d)"

    if ! curl -fL --retry 3 --retry-delay 2 -o "$_tmp/linux-toolchain.tar.xz" "$_url"; then
        rm -rf "$_tmp"
        echo "[init_toolchain] 错误: 工具链下载失败（$_url）" >&2
        return 1 2>/dev/null || exit 1
    fi

    mkdir -p "$_TOOLCHAIN_PATH"
    if ! tar -xJf "$_tmp/linux-toolchain.tar.xz" -C "$_tmp" --strip-components=1; then
        rm -rf "$_tmp"
        echo "[init_toolchain] 错误: 工具链解压失败" >&2
        return 1 2>/dev/null || exit 1
    fi

    # 删除压缩包
    rm -f "$_tmp/linux-toolchain.tar.xz"

    # 将解出的 common/pi32/pi32v2/q32s/README.md 移入 toolchain 目录
    for _item in "$_tmp"/*; do
        [ -e "$_item" ] && mv "$_item" "$_TOOLCHAIN_PATH"/
    done
    rm -rf "$_tmp"   # 删除临时目录

    echo "[init_toolchain] 工具链已就绪: $_TOOLCHAIN_PATH"
else
    echo "[init_toolchain] 工具链已存在: $_TOOLCHAIN_PATH"
fi

# ---------------------------------------------------------------------------
# 步骤二：检测 jlisp 可执行文件，没有则从 GitHub 下载最新 release
# ---------------------------------------------------------------------------
_JLISP_BIN="$AC63N_TOOLS_PATH/jlisp"
if [ -x "$_JLISP_BIN" ]; then
    echo "[init_toolchain] jlisp 已存在: $_JLISP_BIN"
else
    echo "[init_toolchain] 未找到 jlisp，开始从 GitHub 下载最新 release ..."
    _api="https://api.github.com/repos/Z2Z-GuGu/jlisp/releases/latest"
    _rel="$(curl -fsSL --retry 3 --retry-delay 2 "$_api" 2>/dev/null || true)"

    # 提取 linux-amd64 资产的下载地址（排除校验和/签名类文件）
    _asset="$(printf '%s' "$_rel" \
        | grep -oE '"browser_download_url": *"[^"]*linux-amd64[^"]*"' \
        | grep -viE '\.(md5|sha256|sha1|sig|asc|txt)"' \
        | sed -E 's/"browser_download_url": *"([^"]*)"/\1/' \
        | head -1)"

    if [ -z "$_asset" ]; then
        echo "[init_toolchain] 错误: 无法从 GitHub 获取 jlisp linux-amd64 下载地址（$_api）" >&2
        return 1 2>/dev/null || exit 1
    fi

    _tmp="$(mktemp -d)"
    if ! curl -fL --retry 3 --retry-delay 2 -o "$_tmp/jlisp_asset" "$_asset"; then
        rm -rf "$_tmp"
        echo "[init_toolchain] 错误: jlisp 下载失败（$_asset）" >&2
        return 1 2>/dev/null || exit 1
    fi

    # 若下载的是压缩包则解压后取其中可执行文件，否则直接用
    _found=""
    case "$_asset" in
        *.tar.gz|*.tar.xz|*.tgz|*.zip)
            mkdir -p "$_tmp/x"
            if ! tar -xf "$_tmp/jlisp_asset" -C "$_tmp/x" 2>/dev/null && ! unzip -q -o "$_tmp/jlisp_asset" -d "$_tmp/x" 2>/dev/null; then
                rm -rf "$_tmp"
                echo "[init_toolchain] 错误: jlisp 压缩包解压失败（$_asset）" >&2
                return 1 2>/dev/null || exit 1
            fi
            _found="$(find "$_tmp/x" -type f -name 'jlisp*' -executable 2>/dev/null | head -1)"
            [ -z "$_found" ] && _found="$(find "$_tmp/x" -type f -executable 2>/dev/null | head -1)"
            ;;
        *)
            _found="$_tmp/jlisp_asset"
            ;;
    esac

    if [ -z "$_found" ]; then
        rm -rf "$_tmp"
        echo "[init_toolchain] 错误: 未在 release 资产中找到 jlisp 可执行文件（$_asset）" >&2
        return 1 2>/dev/null || exit 1
    fi

    chmod +x "$_found"
    cp "$_found" "$_JLISP_BIN"
    rm -rf "$_tmp"

    echo "[init_toolchain] 已就绪: $_JLISP_BIN"
fi

# ---------------------------------------------------------------------------
# 步骤三：把 AC63N_SDK_PATH / AC63N_TOOLS_PATH 写入 ~/.bashrc（若缺失）
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

# 写入 AC63N_TOOLS_PATH（依赖 $AC63N_SDK_PATH）
if ! _has_line "export AC63N_TOOLS_PATH=\"\$AC63N_SDK_PATH/tools/linux\"" \
   && ! grep -q 'AC63N_TOOLS_PATH=' "$_BASHRC" \
   && ! grep -q 'AC63N_TOOLCHAIN_PATH=' "$_BASHRC"; then
    printf 'export AC63N_TOOLS_PATH="$AC63N_SDK_PATH/tools/linux"\n' >> "$_BASHRC"
    echo "[init_toolchain] 已添加 AC63N_TOOLS_PATH=\$AC63N_SDK_PATH/tools/linux"
else
    echo "[init_toolchain] AC63N_TOOLS_PATH 已存在于 ~/.bashrc，跳过"
fi

# 若之前写入过旧的 AC63N_TOOLCHAIN_PATH，则删除旧定义，避免残留误导
if grep -q 'AC63N_TOOLCHAIN_PATH=' "$_BASHRC"; then
    sed -i '/export AC63N_TOOLCHAIN_PATH=/d' "$_BASHRC"
    echo "[init_toolchain] 已清除旧的 AC63N_TOOLCHAIN_PATH 定义（改用 AC63N_TOOLS_PATH）"
fi

# ---------------------------------------------------------------------------
# 完成
# ---------------------------------------------------------------------------
echo "[init_toolchain] 完成。"
echo "[init_toolchain] 已在当前 shell 导出："
echo "[init_toolchain]   AC63N_SDK_PATH   = $AC63N_SDK_PATH"
echo "[init_toolchain]   AC63N_TOOLS_PATH = $AC63N_TOOLS_PATH"
echo "[init_toolchain] 新开终端后 ~/.bashrc 中的配置会自动生效。"
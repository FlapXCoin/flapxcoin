#!/usr/bin/env bash
# Takes screenshots of the FlapX GUI wallet on a virtual display (used by CI,
# so GUI changes can be checked without a desktop).
#
# Usage: contrib/ci/gui-screenshots.sh <path to flapx-qt> <output directory>
# Needs: Xvfb, xdotool, ImageMagick (import)
#
# Uses a throwaway data directory with networking off, so no real wallet or
# chain data is involved.
set -u

BIN="$(readlink -f "$1")"
mkdir -p "$2"
OUT="$(cd "$2" && pwd)"
DATA="$(mktemp -d)"
cat > "$DATA/flapx.conf" <<EOF
listen=0
dnsseed=0
connect=0
upnp=0
EOF

Xvfb :99 -screen 0 1280x900x24 >/dev/null 2>&1 &
XVFB=$!
export DISPLAY=:99
sleep 2

"$BIN" -datadir="$DATA" -splash=0 > "$OUT/flapx-qt.log" 2>&1 &
APP=$!

cleanup() {
    kill "$APP" 2>/dev/null; sleep 3; kill -9 "$APP" 2>/dev/null
    kill "$XVFB" 2>/dev/null
}

# Wait for the main window
WID=""
for _ in $(seq 1 90); do
    WID="$(xdotool search --onlyvisible --name '^FlapX' 2>/dev/null | head -n1)"
    [ -n "$WID" ] && break
    sleep 2
done
if [ -z "$WID" ]; then
    echo "The wallet window did not appear"
    import -window root "$OUT/00-screen.png"
    cleanup
    exit 1
fi
sleep 8 # let it finish loading

focus() { xdotool windowfocus --sync "$WID" 2>/dev/null || true; }
shot()  { sleep 2; import -window "$1" "$OUT/$2.png" && echo "saved $2.png"; }

# Pages: Alt+1 .. Alt+7
names=(overview send receive transactions addressbook links network)
for n in 1 2 3 4 5 6 7; do
    focus
    xdotool key "alt+$n"
    shot "$WID" "$(printf '%02d' "$n")-${names[$((n-1))]}"
    if [ "$n" = 6 ]; then   # Links page: also its second tab (Block Explorers)
        xdotool key ctrl+Tab
        shot "$WID" "06b-links-explorer"
    fi
done

# Windows opened from the menus. Args: title regex, file name, keys...
dialog() {
    local title="$1" file="$2"; shift 2
    focus
    for k in "$@"; do xdotool key "$k"; sleep 0.7; done
    sleep 2
    local d
    d="$(xdotool search --onlyvisible --name "$title" 2>/dev/null | head -n1)"
    if [ -n "$d" ]; then
        shot "$d" "$file"
        xdotool windowfocus --sync "$d" 2>/dev/null || true
    else
        echo "Window '$title' not found"
        import -window root "$OUT/$file-screen.png"
    fi
    xdotool key Escape
    sleep 1
}
dialog '^Options$'        10-options    alt+s o
dialog 'PIR Calculator'   11-calculator alt+s p
dialog 'Debug window'     12-debug      alt+h d
dialog '^About FlapX'     13-about      alt+h a

cleanup

# Optionally print the screenshots into the log as base64, for reviewers who
# can read the build log but can't download artifacts
if [ "${PRINT_SCREENSHOTS_BASE64:-0}" = 1 ]; then
    for f in "$OUT"/*.png; do
        echo "=== BEGIN SCREENSHOT $(basename "$f") ==="
        base64 -w 1000 "$f"
        echo "=== END SCREENSHOT ==="
    done
fi
exit 0

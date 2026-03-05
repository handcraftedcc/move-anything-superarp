#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
MOVE_ANYTHING_SRC="${MOVE_ANYTHING_SRC:-/Volumes/ExtFS/charlesvestal/github/move-everything-parent/move-anything/src}"
BIN="$ROOT_DIR/build/tests/test_superarp_modifier_trigger"

mkdir -p "$(dirname "$BIN")"

cc -std=c11 -Wall -Wextra -Werror \
  -I"$MOVE_ANYTHING_SRC" \
  -I"$ROOT_DIR/src" \
  "$ROOT_DIR/tests/test_superarp_modifier_trigger.c" \
  "$ROOT_DIR/src/dsp/superarp.c" \
  -o "$BIN"

"$BIN"

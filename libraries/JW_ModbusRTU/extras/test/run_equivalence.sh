#!/usr/bin/env bash
# N1: el motor extraido en JW_ModbusRTU debe producir exactamente las mismas
# tramas y resultados que JWPLC_ModbusRTU.cpp del package.
#
# Uso: bash run_equivalence.sh <ruta a platform-jwplc/JWPLC/2.1.0/libraries>
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
LIB="$(cd "$HERE/../.." && pwd)"
RS485="$(cd "$LIB/../JW_RS485" && pwd)"
PLATFORM_LIBS="${1:?ruta a platform-jwplc/JWPLC/2.1.0/libraries}"
OUT="$(mktemp -d)"
CXX="${CXX:-g++}"
FLAGS="-std=c++11 -Wall -Wextra -Wno-unused-parameter -O1"

$CXX $FLAGS -I"$HERE/mock" -I"$RS485/src" -I"$LIB/src" \
  "$HERE/test_jw.cpp" "$LIB/src/JW_ModbusRTU.cpp" "$RS485/src/JW_RS485.cpp" -o "$OUT/jw"
$CXX $FLAGS -I"$HERE/mock_jwplc" -I"$PLATFORM_LIBS/JWPLC_ModbusRTU/src" \
  "$HERE/test_jwplc_original.cpp" "$PLATFORM_LIBS/JWPLC_ModbusRTU/src/JWPLC_ModbusRTU.cpp" -o "$OUT/jwplc"

"$OUT/jw" > "$OUT/jw.txt"
"$OUT/jwplc" > "$OUT/jwplc.txt"

if diff -u "$OUT/jwplc.txt" "$OUT/jw.txt"; then
  echo "EQUIVALENCE PASS ($(wc -l < "$OUT/jw.txt") lineas identicas)"
else
  echo "EQUIVALENCE FAIL"
  exit 1
fi

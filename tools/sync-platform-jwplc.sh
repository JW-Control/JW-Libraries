#!/usr/bin/env bash
# Copia las librerías JW_* de este repositorio al package JWPLC (platform-jwplc).
# La usa .github/workflows/sync-platform-jwplc-libraries.yml y se puede correr
# a mano para probar la sincronización en un clon local de platform-jwplc.
#
# Uso: tools/sync-platform-jwplc.sh <carpeta libraries del package> "<LIBS>" "<EXCLUIR>"
#   LIBS     nombres de librería separados por espacios (LIBRARIES del .properties)
#   EXCLUIR  rutas <Lib>/<ruta> que no se copian (SYNC_EXCLUDE del .properties)
#
# Librerías precompiladas del package (precompiled=full + src/esp32/*.a):
#   - src/esp32/ del package se conserva siempre (aquí no hay binarios).
#   - Si el código de src/ no cambia, se conserva también precompiled=full.
#   - Si cambia, la librería queda en MODO FUENTE (sin precompiled=full), como
#     pide la regla del package durante una alpha: su .a se regenera al cierre.
#     Así nunca quedan un header nuevo y un .a viejo juntos.
# Escribe en $SYNC_REPORT (si está definido) una línea por librería.
set -euo pipefail

DST_ROOT="${1:?falta la carpeta libraries del package}"
LIBS="${2:?faltan las librerías}"
EXCLUDE="${3:-}"
REPORT="${SYNC_REPORT:-/dev/null}"

src_hash() {  # hash del código de src/ sin los binarios de src/esp32/
  [ -d "$1/src" ] || { echo none; return; }
  (cd "$1/src" && find . -path ./esp32 -prune -o -type f -print | LC_ALL=C sort | xargs -r sha256sum) | sha256sum | cut -c1-16
}

for LIB in $LIBS; do
  SRC="libraries/$LIB"
  DST="$DST_ROOT/$LIB"
  [ -f "$SRC/library.properties" ] || { echo "ERROR: falta $SRC/library.properties"; exit 1; }

  PRE_LINE=""
  OLD_HASH=none
  if [ -d "$DST" ]; then
    PRE_LINE="$(grep -E '^precompiled=' "$DST/library.properties" || true)"
    OLD_HASH="$(src_hash "$DST")"
    # Borrar todo salvo src/esp32/ (binarios del package).
    find "$DST" -mindepth 1 -maxdepth 1 ! -name src -exec rm -rf {} +
    if [ -d "$DST/src" ]; then
      find "$DST/src" -mindepth 1 -maxdepth 1 ! -name esp32 -exec rm -rf {} +
    fi
  fi
  mkdir -p "$DST"

  # Copiar (sin .git, basura del sistema ni las rutas excluidas).
  (cd "$SRC" && tar cf - --exclude=.git --exclude=.github --exclude=.DS_Store --exclude=Thumbs.db .) | tar xf - -C "$DST"
  for EX in $EXCLUDE; do
    case "$EX" in "$LIB"/*) rm -rf "${DST_ROOT:?}/$EX";; esac
  done

  NEW_HASH="$(src_hash "$DST")"
  MODE="fuente"
  if [ -n "$PRE_LINE" ]; then
    if [ "$OLD_HASH" = "$NEW_HASH" ]; then
      if ! grep -qE '^precompiled=' "$DST/library.properties"; then
        [ -z "$(tail -c1 "$DST/library.properties")" ] || echo >> "$DST/library.properties"
        echo "$PRE_LINE" >> "$DST/library.properties"
      fi
      MODE="precompilada (src sin cambios)"
    else
      sed -i '/^precompiled=/d' "$DST/library.properties"
      MODE="MODO FUENTE (src cambió: regenerar src/esp32/*.a al cierre)"
    fi
  fi
  VERSION="$(grep -E '^version=' "$DST/library.properties" | cut -d= -f2 | tr -d ' \r')"
  echo "$LIB $VERSION: $MODE" | tee -a "$REPORT"
done

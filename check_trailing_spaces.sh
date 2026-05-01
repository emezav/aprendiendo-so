#!/usr/bin/env bash
# Detecta (y opcionalmente corrige) trailing spaces en archivos de texto.
#
# Uso:
#   ./check_trailing_spaces.sh [--fix] [directorio]
#
# Sin argumentos escanea el directorio actual.
# Con --fix elimina los trailing spaces in-place.
# Devuelve exit code 1 si encuentra (o encontró antes de corregir) trailing spaces.

set -e

FIX=0
ROOT="."

for arg in "$@"; do
  case "$arg" in
    --fix) FIX=1 ;;
    *)     ROOT="$arg" ;;
  esac
done

# Extensiones de texto a revisar
EXTENSIONS="c h asm s S md txt Makefile makefile sh"

found=0

for ext in $EXTENSIONS; do
  pattern="*.$ext"
  [ "$ext" = "Makefile" ] || [ "$ext" = "makefile" ] && pattern="$ext"

  while IFS= read -r -d '' file; do
    matches=$(grep -En " +$" "$file" 2>/dev/null || true)
    if [ -n "$matches" ]; then
      if [ "$FIX" -eq 1 ]; then
        sed -i 's/[[:space:]]*$//' "$file"
        echo "CORREGIDO: $file"
      else
        echo "--- $file"
        echo "$matches"
      fi
      found=1
    fi
  done < <(find "$ROOT" -name "$pattern" -not -path "*/.git/*" -print0 2>/dev/null)
done

if [ "$found" -eq 0 ]; then
  echo "OK: no se encontraron trailing spaces."
  exit 0
elif [ "$FIX" -eq 1 ]; then
  echo ""
  echo "Listo: todos los trailing spaces fueron eliminados."
  exit 0
else
  echo ""
  echo "FALLO: se encontraron trailing spaces (ver arriba)."
  exit 1
fi

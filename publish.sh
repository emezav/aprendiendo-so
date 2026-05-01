#!/usr/bin/env bash
# Publica el contenido del repo dev en el repo público para estudiantes.
# Usa git archive, que respeta los export-ignore definidos en .gitattributes.
#
# Uso:
#   ./publish.sh <ruta-o-url-del-repo-público>
#
# Ejemplos:
#   ./publish.sh ../aprendiendo-so
#   ./publish.sh git@github.com:usuario/aprendiendo-so.git

set -e

# En Windows con MSYS2, HOME puede apuntar al directorio interno de MSYS2
# en lugar del perfil de usuario de Windows donde vive .gitconfig.
# cygpath convierte el path de Windows al formato Unix que bash entiende.
if [ -n "$USERPROFILE" ] && command -v cygpath >/dev/null 2>&1; then
  export HOME=$(cygpath -u "$USERPROFILE")
fi

PUBLIC="$1"

if [ -z "$PUBLIC" ]; then
  echo "Uso: $0 <ruta-o-url-del-repo-público>"
  exit 1
fi

# Si el destino es una ruta local, debe ser un repo git ya inicializado.
if [ -d "$PUBLIC" ]; then
  echo "[publish] exportando a directorio local: $PUBLIC"
  # Limpiar contenido anterior para evitar residuos de ejecuciones previas.
  # Se usan solo builtins de bash para evitar que Windows find.exe tome
  # precedencia sobre el find de MSYS2 en el PATH.
  (shopt -s dotglob nullglob; cd "$PUBLIC" && for item in *; do
    [ "$item" != ".git" ] && rm -rf "$item"
  done)
  git archive HEAD | tar -x -C "$PUBLIC"
  cd "$PUBLIC"
  git add -A
  git diff --cached --quiet && echo "[publish] sin cambios nuevos" && exit 0
  git commit -m "sync: $(date '+%Y-%m-%d %H:%M')"
  git push
else
  # Destino remoto: clonar en temporal, exportar, empujar.
  echo "[publish] destino remoto detectado"
  TMPDIR=$(mktemp -d)
  git clone "$PUBLIC" "$TMPDIR"
  git archive HEAD | tar -x -C "$TMPDIR"
  cd "$TMPDIR"
  git add -A
  git diff --cached --quiet && echo "[publish] sin cambios nuevos" && exit 0
  git commit -m "sync: $(date '+%Y-%m-%d %H:%M')"
  git push
  rm -rf "$TMPDIR"
fi

echo "[publish] listo."

# Guía interna JW Control para crear, publicar y actualizar librerías Arduino

Este documento resume el flujo recomendado para desarrollar una librería en el monorepo `JW-Libraries`, publicarla en GitHub como subrepo independiente, automatizar sus releases y añadirla al Arduino Library Manager.

---

## 1. Convención recomendada

### Monorepo principal
- Repositorio: `JW-Control/JW-Libraries`
- Estructura sugerida:

```text
JW-Libraries/
├── .github/
│   └── workflows/
├── libraries/
│   ├── JW_MatrixButtons/
│   ├── JW_DWIN_RS485/
│   └── JW_FRAM/
└── README.md
```

### Convención de nombres
Usar siempre prefijo `JW_` para facilitar búsqueda y uniformidad.

Librerías previstas:
- `JW_MatrixButtons`
- `JW_DWIN_RS485`
- `JW_FRAM`

Subrepos públicos esperados:
- `JW-Control/JW_MatrixButtons`
- `JW-Control/JW_DWIN_RS485`
- `JW-Control/JW_FRAM`

---

## 2. Estructura mínima que debe tener cada librería

Cada carpeta dentro de `libraries/` debe ser una librería Arduino completa por sí sola:

```text
JW_<LIB>/
├── src/
├── examples/
├── library.properties
├── README.md
├── CHANGELOG.md
├── keywords.txt
└── LICENSE
```

### Recomendaciones
- `LICENSE`: usar MIT.
- `library.properties`: en la raíz de la librería.
- `version=`: seguir SemVer (`MAJOR.MINOR.PATCH`), por ejemplo `1.0.0`, `1.1.0`, `2.0.0`.
- `maintainer`: usar siempre `JW Control <jw.control.peru@gmail.com>`.

Ejemplo base de `library.properties`:

```properties
name=JW_MatrixButtons
version=1.0.0
author=JW Control
maintainer=JW Control <jw.control.peru@gmail.com>
sentence=Lectura de matriz de botones con debounce, eventos y repetición.
paragraph=Librería para leer una matriz de botones con debounce, eventos press/release/repeat y helpers para navegación tipo HMI/PLC.
category=Signal Input/Output
url=https://github.com/JW-Control/JW_MatrixButtons
architectures=*
includes=JW_MatrixButtons.h
depends=
```

---

## 3. Flujo recomendado de trabajo

### Filosofía
1. Todo el desarrollo se hace en el monorepo `JW-Libraries`.
2. Cada librería tiene su propio subrepo de distribución.
3. El release se crea automáticamente **solo cuando se actualiza `library.properties`**.
4. Antes de tocar `library.properties`, se hacen todos los cambios de código, README, ejemplos, changelog, keywords, etc.
5. Al final se actualiza `version=` y recién ahí corre el workflow.

### Secuencia diaria recomendada
1. Editar archivos de la librería en `JW-Libraries/libraries/JW_<LIB>/`
2. Probar localmente.
3. Actualizar:
   - `README.md`
   - `CHANGELOG.md`
   - `keywords.txt`
   - otros archivos necesarios
4. Finalmente actualizar `version=` en `library.properties`.
5. Hacer commit y push.
6. GitHub Actions se encarga de:
   - hacer subtree split
   - sincronizar el subrepo
   - crear tag `vX.Y.Z`
   - crear release automáticamente

---

## 4. Crear una nueva librería en el monorepo

### Paso 1: Crear carpeta de librería
Ejemplo:

```text
libraries/JW_DWIN_RS485/
```

### Paso 2: Añadir estructura mínima
Crear:
- `src/`
- `examples/`
- `library.properties`
- `README.md`
- `CHANGELOG.md`
- `keywords.txt`
- `LICENSE`

### Paso 3: Llenar los metadatos
Ajustar:
- `name=`
- `url=`
- `includes=`
- `sentence=`
- `paragraph=`

### Paso 4: Primer commit local en monorepo

```bash
git add .
git commit -m "Add JW_DWIN_RS485 library scaffold"
git push
```

---

## 5. Crear el subrepo de distribución en GitHub

Para cada nueva librería, crear un repo público independiente.

Ejemplos:
- `JW-Control/JW_MatrixButtons`
- `JW-Control/JW_DWIN_RS485`
- `JW-Control/JW_FRAM`

### Recomendación
El subrepo puede crearse vacío o con README/LICENSE, pero el flujo actual ya sobrescribe el contenido principal mediante el workflow.

---

## 6. Secret necesario para automatización

En `JW-Libraries` debe existir el secret:

```text
DIST_PUSH_TOKEN
```

Este token debe tener acceso a:
- `JW-Control/JW-Libraries`
- todos los subrepos de distribución

Permisos recomendados del token:
- Metadata: Read
- Contents: Read and write

---

## 7. Plantilla general del workflow por librería

Cada librería tendrá su propio workflow en:

```text
.github/workflows/sync-<nombre>.yml
```

Ejemplo para `JW_MatrixButtons`:

```yaml
name: Sync JW_MatrixButtons + Auto Release

on:
  push:
    branches: ["main"]
    paths:
      - "libraries/JW_MatrixButtons/library.properties"
      - ".github/workflows/sync-jw-matrixbuttons.yml"
  workflow_dispatch:

jobs:
  sync:
    runs-on: ubuntu-latest

    permissions:
      contents: write

    steps:
      - name: Checkout monorepo
        uses: actions/checkout@v4
        with:
          fetch-depth: 0
          persist-credentials: false

      - name: Configure git identity
        run: |
          git config user.name "JW-Control CI"
          git config user.email "jw.control.peru@gmail.com"

      - name: Extract version from library.properties
        id: ver
        shell: bash
        run: |
          PROP="libraries/JW_MatrixButtons/library.properties"
          if [ ! -f "$PROP" ]; then
            echo "ERROR: $PROP not found"
            exit 1
          fi

          VERSION=$(grep -E '^version=' "$PROP" | head -n1 | cut -d'=' -f2 | tr -d ' \r')
          if [ -z "$VERSION" ]; then
            echo "ERROR: version= not found in $PROP"
            exit 1
          fi

          echo "version=$VERSION" >> "$GITHUB_OUTPUT"
          echo "tag=v$VERSION" >> "$GITHUB_OUTPUT"

      - name: Add distribution remote (PAT)
        shell: bash
        env:
          TOKEN: ${{ secrets.DIST_PUSH_TOKEN }}
        run: |
          if [ -z "$TOKEN" ]; then
            echo "ERROR: DIST_PUSH_TOKEN is empty/not set"
            exit 1
          fi

          git remote remove dist || true
          git remote add dist "https://x-access-token:${TOKEN}@github.com/JW-Control/JW_MatrixButtons.git"

      - name: Subtree split
        id: split
        shell: bash
        run: |
          git subtree split --prefix=libraries/JW_MatrixButtons -b split-jw-matrixbuttons
          SHA=$(git rev-parse split-jw-matrixbuttons)
          echo "sha=$SHA" >> "$GITHUB_OUTPUT"

      - name: Push subtree to subrepo (main)
        run: |
          git push dist split-jw-matrixbuttons:main --force

      - name: Create GitHub Release in subrepo (only if missing)
        shell: bash
        env:
          GH_TOKEN: ${{ secrets.DIST_PUSH_TOKEN }}
          TAG: ${{ steps.ver.outputs.tag }}
          VERSION: ${{ steps.ver.outputs.version }}
          SHA: ${{ steps.split.outputs.sha }}
        run: |
          set -e

          if gh release view "$TAG" -R "JW-Control/JW_MatrixButtons" >/dev/null 2>&1; then
            echo "Release $TAG already exists. Skipping."
            exit 0
          fi

          gh release create "$TAG" \
            -R "JW-Control/JW_MatrixButtons" \
            --target "$SHA" \
            --title "JW_MatrixButtons $TAG" \
            --notes "Release $TAG (version=$VERSION)."
```

### Qué se debe cambiar al reutilizar la plantilla
Cambiar estas partes:
- `JW_MatrixButtons`
- `JW-Control/JW_MatrixButtons`
- `sync-jw-matrixbuttons.yml`
- el nombre del branch temporal `split-jw-matrixbuttons`
- la ruta `libraries/JW_MatrixButtons/...`

---

## 8. Secuencia para publicar una nueva librería en Arduino Library Manager

### Paso 1: Tener el subrepo listo
Debe contener en la raíz:
- `src/`
- `examples/`
- `library.properties`
- `README.md`
- `LICENSE`

### Paso 2: Crear primera versión
En el monorepo:
- dejar todo listo
- actualizar `version=1.0.0`
- push
- esperar a que el workflow cree el release `v1.0.0`

### Paso 3: Añadir la librería a Arduino Library Manager
1. Hacer fork de `arduino/library-registry`
2. Crear rama de trabajo
3. Editar `repositories.txt`
4. Añadir una línea con la URL del repo, por ejemplo:

```text
https://github.com/JW-Control/JW_MatrixButtons
```

5. Crear Pull Request hacia `arduino/library-registry`
6. Esperar validación del bot y merge

### Paso 4: Esperar indexación
Arduino revisa el listado y sus releases periódicamente. Una vez aceptado el repo, las nuevas versiones compatibles se publican automáticamente sin volver a hacer otro PR, siempre que se cree una nueva release con versión válida.

---

## 9. Publicar nuevas versiones una vez que la librería ya está en Arduino

No se vuelve a enviar PR al registry por cada versión nueva.

La secuencia correcta es:
1. Hacer cambios en la librería dentro del monorepo.
2. Actualizar `CHANGELOG.md`.
3. Cambiar `version=` en `library.properties`.
4. Hacer push.
5. El workflow sincroniza el subrepo y crea el release nuevo.
6. Arduino Library Manager detecta la nueva release automáticamente.

---

## 10. Cambio de nombre de librerías ya publicadas

### Caso actual: `JWMatrixButtons` → `JW_MatrixButtons`
Esto **no debe hacerse como cambio normal del campo `name`** en `library.properties` de una librería que ya fue aceptada por Arduino Library Manager.

### Motivo
Arduino indica que, para publicaciones nuevas de una librería ya incluida, el campo `name` de `library.properties` **no debe cambiar** respecto al nombre usado al momento de la primera aceptación. Si es necesario cambiar el nombre, debe seguirse un proceso especial indicado en su FAQ. Además, el Library Manager no acepta una librería nueva si el campo `name` ya existe en otra previamente registrada. citeturn771341search0

### Recomendación práctica
Para evitar problemas:
- Mantener `JWMatrixButtons` como librería ya publicada.
- Si se quiere migrar al esquema `JW_`, crear una librería nueva real:
  - nuevo subrepo: `JW-Control/JW_MatrixButtons`
  - nuevo `name=JW_MatrixButtons`
  - nueva publicación al registry
- Opcionalmente dejar la antigua como compatibilidad o marcarla como deprecada en README.

### Conclusión recomendada
**No renombrar la ya publicada dentro del mismo registro.**
Conviene más tratar `JW_MatrixButtons` como una librería nueva.

---

## 11. CHANGELOG.md recomendado

Formato sugerido:

```markdown
# Changelog

All notable changes to this project will be documented in this file.

## [1.0.0] - 2026-02-19
### Added
- Initial release.
- Matrix scanning (rows/columns).
- Debounce handling.
- Press / Release / Repeat events.
- Key repeat acceleration.
- Event queue support.
```

---

## 12. keywords.txt recomendado

Formato:

```text
JW_MatrixButtons	KEYWORD1
begin	KEYWORD2
update	KEYWORD2
getEvent	KEYWORD2
isPressed	KEYWORD2
isReleased	KEYWORD2
EVENT_PRESS	LITERAL1
EVENT_RELEASE	LITERAL1
EVENT_REPEAT	LITERAL1
```

Usar tabulaciones reales entre palabra y tipo.

---

## 13. Checklist rápido antes de bump de versión

### Checklist técnico
- [ ] Código probado
- [ ] README actualizado
- [ ] CHANGELOG actualizado
- [ ] keywords.txt revisado
- [ ] LICENSE presente
- [ ] library.properties correcto
- [ ] version incrementado

### Checklist de distribución
- [ ] Subrepo existe
- [ ] Secret `DIST_PUSH_TOKEN` vigente
- [ ] Workflow correcto
- [ ] Release anterior verificado

---

## 14. Resumen ejecutivo corto

### Para una librería nueva
1. Crear carpeta en `libraries/`
2. Completar estructura mínima
3. Crear subrepo público
4. Añadir workflow dedicado
5. Subir `version=1.0.0`
6. Verificar release automático
7. Hacer PR a `arduino/library-registry`

### Para una versión nueva
1. Modificar código en monorepo
2. Al final cambiar `version=` en `library.properties`
3. Push a `main`
4. Verificar release automático
5. Esperar indexación de Arduino


# git-toolkit

Aplicación nativa multiplataforma para gestionar repositorios Git con múltiples cuentas y proveedores (GitHub, GitLab, Gitea, Forgejo).

## Estado

En desarrollo. Consulta el [plan de evolución](../../.claude/plans/magical-mapping-pine.md) para detalles.

## Características previstas

- **Asistente de configuración**: Añade cuentas y repos de forma guiada, sin editar JSON
- **Tres métodos de autenticación**: HTTPS+GCM (navegador), Token (PAT), SSH
- **Panel de estado**: Ve de un vistazo qué repos necesitan pull, están sucios o tienen conflictos
- **Pull selectivo**: Por repositorio, por fuente o global
- **Multiplataforma nativa**: SwiftUI (macOS), Win32 (Windows), GTK4 (Linux)
- **CLI incluido**: `git-toolkit sync`, `git-toolkit status`, `git-toolkit pull`

## Arquitectura

```text
core/     -> Librería C++ compartida (config, credenciales, git, SSH, estado)
cli/      -> Frontend CLI en C++
gui/      -> GUIs nativas por plataforma
  macos/  -> SwiftUI + bridge C
  windows/-> Win32 + Common Controls
  linux/  -> GTK4 via gtkmm
```

## Configuración

El fichero de configuración se ubica en:

| Plataforma | Ruta |
| ---------- | ---- |
| Linux / macOS / Git Bash | `~/.config/git-toolkit/git-toolkit.json` |
| WSL2 | `/mnt/c/Users/<user>/.config/git-toolkit/git-toolkit.json` |

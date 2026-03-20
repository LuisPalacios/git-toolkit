# git-toolkit

Aplicacion nativa multiplataforma para gestionar repositorios Git con multiples cuentas y proveedores (GitHub, GitLab, Gitea, Forgejo).

## Estado

En desarrollo. Consulta el [plan de evolucion](../../.claude/plans/magical-mapping-pine.md) para detalles.

## Caracteristicas previstas

- **Asistente de configuracion**: Anade cuentas y repos de forma guiada, sin editar JSON
- **Tres metodos de autenticacion**: HTTPS+GCM (navegador), Token (PAT), SSH
- **Panel de estado**: Ve de un vistazo que repos necesitan pull, estan sucios o tienen conflictos
- **Pull selectivo**: Por repositorio, por fuente o global
- **Multiplataforma nativa**: SwiftUI (macOS), Win32 (Windows), GTK4 (Linux)
- **CLI incluido**: `git-toolkit sync`, `git-toolkit status`, `git-toolkit pull`

## Arquitectura

```text
core/     -> Libreria C++ compartida (config, credenciales, git, SSH, estado)
cli/      -> Frontend CLI en C++
gui/      -> GUIs nativas por plataforma
  macos/  -> SwiftUI + bridge C
  windows/-> Win32 + Common Controls
  linux/  -> GTK4 via gtkmm
```

## Configuracion

El fichero de configuracion se ubica en:

| Plataforma | Ruta |
| ---------- | ---- |
| Linux / macOS / Git Bash | `~/.config/git-toolkit/git-toolkit.json` |
| WSL2 | `/mnt/c/Users/<user>/.config/git-toolkit/git-toolkit.json` |

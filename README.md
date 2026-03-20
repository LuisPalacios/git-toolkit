# git-toolkit

Gestión de repositorios Git con múltiples cuentas y proveedores (GitHub, GitLab, Gitea, Forgejo).

Compatible con macOS, Linux, Windows (Git Bash) y WSL2.

Más información: [Git Multicuenta](https://luispa.com/posts/2024-09-21-git-multicuenta/).

## git-toolkit (app)

> **En desarrollo**

Aplicación nativa con asistente de configuración, panel de estado y pull selectivo.
CLI incluido: `git-toolkit sync`, `git-toolkit status`, `git-toolkit pull`.

[Documentación](git-toolkit/docs/README.md)

## git-config-repos.sh

Script Bash para configurar repositorios Git de forma automática. Soporta múltiples cuentas y proveedores con autenticación HTTPS+GCM o SSH. Para usuarios avanzados y servidores headless.

[Documentación](git-config-repos/docs/README.md)

## git-status-pull.sh

Script Bash que verifica el estado de sincronización de todos los repositorios Git desde el directorio actual. Puede hacer pull automático donde sea seguro.

[Documentación](git-status-pull/docs/README.md)

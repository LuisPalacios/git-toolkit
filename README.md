# git-toolkit

Gestion de repositorios Git con multiples cuentas y proveedores (GitHub, GitLab, Gitea, Forgejo).

Compatible con macOS, Linux, Windows (Git Bash) y WSL2.

Mas informacion: [Git Multicuenta](https://luispa.com/posts/2024-09-21-git-multicuenta/).

## git-toolkit (app)

> **En desarrollo**

Aplicacion nativa con asistente de configuracion, panel de estado y pull selectivo.
CLI incluido: `git-toolkit sync`, `git-toolkit status`, `git-toolkit pull`.

[Documentacion](git-toolkit/docs/README.md)

## git-config-repos.sh

Script Bash para configurar repositorios Git de forma automatica. Soporta multiples cuentas y proveedores con autenticacion HTTPS+GCM o SSH. Para usuarios avanzados y servidores headless.

[Documentacion](git-config-repos/docs/README.md)

## git-status-pull.sh

Script Bash que verifica el estado de sincronizacion de todos los repositorios Git desde el directorio actual. Puede hacer pull automatico donde sea seguro.

[Documentacion](git-status-pull/docs/README.md)

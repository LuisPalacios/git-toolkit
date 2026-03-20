# Herramientas para repositorios Git

Scripts de apoyo para trabajar con Git en entorno de múltiples cuentas con uno o más proveedores Git (GitHub, GitLab, Gitea).

Soportan dos opciones de autenticación: 

1. HTTPS + Git Credential Manager
2. SSH multicuenta. La primera, HTTPS + Git Credential Manager, es la que suelo usar cuando trabajo en un Desktop con UI porque es compatible con el CLI y/o herramientas GUI tipo Visual Studio, VSCode, Git Desktop, Gitkraken, etc.

La segunda opción, SSH multicuenta, la suelo usar en equipos linux "headless", servidores a los que conecto en remoto vía (CLI o VSCode remote) y necesito que clonen repositorios y trabajen sobre ellos. Echa un ojo al apunte [Git Multicuenta](https://luispa.com/posts/2024-09-21-git-multicuenta/).

## Compatibilidad de plataformas

Todos los scripts son `.sh` y funcionan en las siguientes plataformas sin modificación:

| Plataforma | Detección | Comando git |
| ---------- | --------- | ----------- |
| Linux nativo | por defecto | `git` |
| macOS | `$OSTYPE == darwin*` | `git` |
| WSL2 | `/proc/version` contiene "Microsoft"/"WSL" | `git.exe` |
| Git Bash (Windows) | `$MSYSTEM` definida | `git` |

## `git-config-repos.sh`

Simplifica la configuración de Git multicuenta. Lee y analiza el archivo `~/.config/git-config-repos/git-config-repos.json`, de donde obtiene parámetros globales de GIT y parámetros específicos para el tipo de credenciales, cuentas y repositorios.

Para cada uno de los repositorios:

- Verifica los credenciales y los guarda en el almacén local
- Clona el repo bajo el directorio de la cuenta y lo configura
- Si ya existía el repo, revisa la configuración y arregla lo que no esté correcto

El fichero JSON tiene dos claves principales: "global" y "accounts".

- La clave "global" indica cuál es el directorio raíz GIT donde el usuario despliega toda la estructura de directorios y algunos parámetros que servirán para configurar el fichero global `~/.gitconfig` y en el caso de SSH el fichero `~/.ssh/config`. El campo `folder` y `ssh_folder` soportan ruta absoluta (`/home/luis/00.git`) o con tilde (`~/00.git`).
- La clave "accounts" incluye claves para diferentes cuentas en distintos proveedores Git y dentro de dichas cuentas incluye a su vez repositorios.

Este script está probado en Linux, macOS, WSL2 y Git Bash (Windows).

### Dependencias

| Plataforma | Dependencias |
| ---------- | ------------ |
| Linux | `git`, `jq`, `git-credential-manager` (si usa GCM) |
| macOS | `git`, `jq`, `git-credential-manager` (`brew install --cask git-credential-manager`) |
| WSL2 | `jq` (en WSL2), `git.exe` + `cmd.exe` + `git-credential-manager.exe` (en Windows host) |
| Git Bash | `git`, `jq.exe`, `git-credential-manager` (incluido con Git for Windows >= 2.39) |

### Ejecución

```bash
chmod +x git-config-repos.sh
./git-config-repos.sh
```

## `git-status-pull.sh`

Este script verifica el estado de múltiples repositorios Git a partir del directorio actual (desde donde lo ejecutes). Su objetivo es informar al usuario sobre qué repositorios necesitan un pull para estar sincronizados con su upstream.

Si se proporciona el argumento `pull`, el script puede hacer pull automáticamente. Además es capaz de proporcionar información detallada sobre cada repositorio, cuando no se puede hacer pull automáticamente, informando de la razón por la que el repositorio no está limpio y necesita ser revisado. Soporta el modo verbose (`-v`) para dar dicha información más detallada.

```bash
./git-status-pull.sh          # Muestra estado de todos los repos
./git-status-pull.sh pull     # Hace pull automático donde sea seguro
./git-status-pull.sh -v       # Salida detallada
./git-status-pull.sh -v pull  # Detallado + pull automático
```

## Patrones de configuración soportados

El fichero `git-config-repos.jsonc` incluido en este repositorio es una **plantilla comentada** con ejemplos de todos los patrones posibles:

- **Cuenta estándar**: GitHub, GitLab, Gitea con GCM o SSH
- **Dos cuentas en el mismo servidor**: diferenciadas por el path de la URL
- **Cuenta dividida (split)**: misma org/usuario, distintas carpetas locales — para organizar repos en subdirectorios separados
- **Cuenta readonly/cross-org**: `url` sin usuario al final + nombre de repo con slash (`"org/repo"`) — para clonar repos de organizaciones ajenas
- **Cuenta mínima**: solo los campos requeridos (sin `gcm_provider` ni SSH)
- **Override por repo**: `name`, `email`, `folder` y `credential_type` a nivel de repositorio individual

## JSON Schema

El fichero `git-config-repos.schema.json` define un [JSON Schema](https://json-schema.org/) (draft 2020-12) para validar y autocompletar el archivo de configuración.

Para activar la validación en tu editor, añade `$schema` al inicio de tu `git-config-repos.json`:

```json
{
    "$schema": "https://raw.githubusercontent.com/LuisPalacios/git-toolkit/main/git-config-repos.schema.json",
    "global": { ... },
    "accounts": { ... }
}
```

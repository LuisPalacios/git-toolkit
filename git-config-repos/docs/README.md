# git-config-repos.sh

Script Bash para configurar repositorios Git de forma automatica, soportando multiples cuentas y proveedores (GitHub, GitLab, Gitea) con autenticacion HTTPS+GCM o SSH.

## Ejecucion

```bash
chmod +x git-config-repos.sh
./git-config-repos.sh            # Ejecutar
./git-config-repos.sh --dry-run  # Ver que haria sin ejecutar
./git-config-repos.sh --help     # Ayuda
```

## Compatibilidad de plataformas

| Plataforma | Deteccion | Comando git |
| ---------- | --------- | ----------- |
| Linux nativo | por defecto | `git` |
| macOS | `$OSTYPE == darwin*` | `git` |
| WSL2 | `/proc/version` contiene "Microsoft"/"WSL" | `git.exe` |
| Git Bash (Windows) | `$MSYSTEM` definida | `git` |

## Configuracion

El script lee el fichero JSON de configuracion en:

| Plataforma | Ruta |
| ---------- | ---- |
| Linux / macOS / Git Bash | `~/.config/git-config-repos/git-config-repos.json` |
| WSL2 | `/mnt/c/Users/<user>/.config/git-config-repos/git-config-repos.json` |

El fichero `git-config-repos.jsonc` en este directorio es una plantilla comentada con ejemplos de todos los patrones soportados.

El fichero `git-config-repos.schema.json` define un [JSON Schema](https://json-schema.org/) para validar y autocompletar la configuracion en editores.

## Dependencias

| Plataforma | Dependencias |
| ---------- | ------------ |
| Linux | `git`, `jq`, `git-credential-manager` (si usa GCM) |
| macOS | `git`, `jq`, `git-credential-manager` |
| WSL2 | `jq` (en WSL2), `git.exe` + `cmd.exe` + `git-credential-manager.exe` (Windows host) |
| Git Bash | `git`, `jq.exe`, `git-credential-manager` (incluido con Git for Windows >= 2.39) |

## Estructura del JSON

```text
global:
  folder          - directorio raiz para los repos (soporta ~/ruta o absoluta)
  credential_ssh: enabled, ssh_folder
  credential_gcm: enabled, helper, credentialStore

accounts.<Clave>:
  url, username, folder, name, email
  gcm_provider, gcm_useHttpPath  (opcionales)
  ssh_host, ssh_hostname, ssh_type  (opcionales, solo si algun repo usa ssh)
  repos.<NombreRepo>:
    credential_type: "gcm" | "ssh"
    name, email     (opcionales, sobreescriben los de la cuenta)
    folder          (opcional: absoluta o relativa al directorio de la cuenta)
```

## Patrones de configuracion soportados

- **Cuenta estandar**: GitHub, GitLab, Gitea con GCM o SSH
- **Dos cuentas en el mismo servidor**: diferenciadas por el path de la URL
- **Cuenta dividida (split)**: misma org/usuario, distintas carpetas locales
- **Cuenta readonly/cross-org**: `url` sin usuario al final + nombre de repo con slash (`"org/repo"`)
- **Cuenta minima**: solo los campos requeridos (sin `gcm_provider` ni SSH)
- **Override por repo**: `name`, `email`, `folder` y `credential_type` a nivel de repositorio individual

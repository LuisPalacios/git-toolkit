# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Workflow Orchestration

**Priority order when rules conflict**: Correctness > Simplicity > Elegance.

### 1. Plan First

- Enter plan mode for ANY non-trivial task (3+ steps or architectural decisions)
- If something goes sideways, STOP and re-plan immediately — don't keep pushing
- Write detailed specs upfront to reduce ambiguity

### 2. Subagent Strategy

- Use subagents liberally to keep main context window clean
- Offload research, exploration, and parallel analysis to subagents
- One task per subagent for focused execution

### 3. Verify Before Done

- Never mark a task complete without proving it works
- Test scripts with `bash -n` (syntax check) and `shellcheck` when available
- Validate config templates render correctly before committing
- Diff behavior between main and your changes when relevant

### 4. Autonomous Bug Fixing

- When given a bug report: just fix it. Don't ask for hand-holding
- Point at logs, errors, failing tests — then resolve them
- Zero context switching required from the user

### 5. Learn from Corrections

- After corrections from the user: save a `feedback` memory via the memory system
- Do NOT maintain a separate lessons file — use `.claude/projects/.../memory/` exclusively

## Task Management

1. **Plan First**: Write a plan with checkable items before starting
2. **Verify Plan**: Check in before starting implementation
3. **Track Progress**: Mark items complete as you go
4. **Explain Changes**: High-level summary at each step

## Core Principles

- **Simplicity First**: Make every change as simple as possible. Impact minimal code.
- **No Laziness**: Find root causes. No temporary fixes. Senior developer standards.
- **Minimal Impact**: Changes should only touch what's necessary. Avoid introducing bugs.
- **Skills first**: Check if a skill exists (`/fixing-markdown`, etc.) before doing the work manually
- **Self-improve**: When a skill fails or has gaps, update its `SKILL.md` with the fix
- **Zero entropy**: Never create files outside defined structure

## What this repo is

A collection of Bash scripts to manage Git multi-account environments across multiple Git providers (GitHub, GitLab, Gitea). Supports two authentication methods: HTTPS + Git Credential Manager (GCM) and SSH multi-account. Compatible with Linux, macOS, WSL2, and Git Bash on Windows — all via `.sh` scripts, no platform-specific wrappers.

## Scripts

### `git-config-repos.sh`
Reads `~/.config/git-config-repos/git-config-repos.json` and for each configured account/repo: verifies credentials, clones repos that don't exist, and fixes configuration of existing ones. Supports GCM and SSH credential types per-repo.

**Run:**
```bash
chmod +x git-config-repos.sh
./git-config-repos.sh
```

**Dependencies:**

| Platform | Required |
| -------- | -------- |
| Linux | `git`, `jq`, `git-credential-manager` (if using GCM) |
| macOS | `git`, `jq`, `git-credential-manager` |
| WSL2 | `jq` (in WSL2), `git.exe` + `cmd.exe` + `git-credential-manager.exe` (Windows host) |
| Git Bash | `git`, `jq.exe`, `git-credential-manager` (bundled with Git for Windows >= 2.39) |

### `git-status-pull.sh`
Scans all `.git` directories from the current working directory and reports sync status with upstream. Can auto-pull if safe to do so.

```bash
./git-status-pull.sh          # Check status
./git-status-pull.sh pull     # Auto-pull where safe
./git-status-pull.sh -v       # Verbose output
./git-status-pull.sh -v pull  # Both
```

## Platform detection

Both scripts share the same detection block, setting `PLATFORM` (`wsl2` | `gitbash` | `macos` | `linux`) and `cmdgit` (`git.exe` on WSL2, `git` elsewhere). Git Bash is detected first via `$MSYSTEM` before the `/proc/version` check.

## JSON configuration (`git-config-repos.json`)

The file `git-config-repos.json` in this repo is an **annotated example** (it contains JS-style comments and must have them stripped before use). The actual config must be placed at:

| Platform | Path |
| -------- | ---- |
| Linux / macOS / Git Bash | `~/.config/git-config-repos/git-config-repos.json` |
| WSL2 | `/mnt/c/Users/<user>/.config/git-config-repos/git-config-repos.json` |

### JSON structure

```text
global:
  folder          - root dir for all git repos (supports ~/path or absolute)
  credential_ssh: enabled, ssh_folder (supports ~/path)
  credential_gcm: enabled, helper, credentialStore
                  (wincredman=Windows, keychain=macOS, secretservice=Linux)

accounts.<AccountKey>:
  url             - provider URL + user path; omit user path for cross-org clones
  username, folder, name, email
  gcm_provider, gcm_useHttpPath  (optional — skipped if absent)
  ssh_host, ssh_hostname, ssh_type  (optional — required only if a repo uses ssh)
  repos.<RepoName>:
    credential_type: "gcm" | "ssh"
    name, email     (optional per-repo overrides)
    folder          (optional: absolute or relative to account folder)
```

### Account patterns documented in the example config

- **Standard**: Gitea/GitHub with GCM or SSH
- **Two accounts same server**: different user path in `url`
- **Split account**: same org/username, different local `folder`
- **Readonly / cross-org**: `url` without user path + repo name as `"org/repo"`
- **Minimal account**: only required fields; `gcm_provider` and SSH fields are optional

## Windows specifics

**WSL2:** Uses `git.exe` (Windows Git) to integrate with Windows Credential Manager and avoid WSL2 filesystem slowness. Path conversion `/mnt/c/...` → `C:\...` is done internally via `convert_wsl_to_windows_path()` only at clone time.

**Git Bash:** Uses native `git`. Paths stay in POSIX format throughout. Windows Credential Manager is accessed via `cmd.exe cmdkey`, same mechanism as WSL2.

## Credential types

- **GCM (HTTPS):** Uses Git Credential Manager. Credentials are stored in the OS keychain (Windows Credential Manager / macOS Keychain / Linux Secret Service). The script triggers browser-based authentication when credentials are missing.
- **SSH:** Generates per-account ed25519 (or configured type) key pairs at `~/.ssh/<ssh_host>-sshkey`. Adds an `Include` directive to `~/.ssh/config` pointing to `~/.ssh/git-config-repos` which is regenerated on each run.

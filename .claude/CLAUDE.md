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

A collection of Bash and PowerShell scripts to manage Git multi-account environments across multiple Git providers (GitHub, GitLab, Gitea). It supports two authentication methods: HTTPS + Git Credential Manager (GCM) and SSH multi-account.

## Scripts

### `git-config-repos.sh`
Reads `~/.config/git-config-repos/git-config-repos.json` and for each configured account/repo: verifies credentials, clones repos that don't exist, and fixes configuration of existing ones. Supports GCM and SSH credential types per-repo.

**Run:**
```bash
chmod +x git-config-repos.sh
./git-config-repos.sh
```

**Dependencies:** `git`, `jq`. On WSL2: also `git.exe`, `cmd.exe`, `wslpath`, `git-credential-manager.exe`. On macOS/Linux with GCM: `git-credential-manager`.

### `git-status-pull.sh`
Scans all `.git` directories from the current working directory and reports sync status with upstream. Can auto-pull if safe to do so.

```bash
./git-status-pull.sh          # Check status
./git-status-pull.sh pull     # Auto-pull where safe
./git-status-pull.sh -v       # Verbose output
./git-status-pull.sh -v pull  # Both
```

### `git-status-pull.ps1`
PowerShell equivalent of `git-status-pull.sh` for native Windows (not WSL2).

```powershell
.\git-status-pull.ps1           # Check status
.\git-status-pull.ps1 -Pull     # Auto-pull where safe
.\git-status-pull.ps1 -Verbose  # Verbose output
```

## JSON configuration (`git-config-repos.json`)

The file `git-config-repos.json` in this repo is an **annotated example** (it contains JS-style comments and must have them stripped before use). The actual config must be placed at:
- Linux/macOS: `~/.config/git-config-repos/git-config-repos.json`
- WSL2: `C:\Users\<user>\.config\git-config-repos\git-config-repos.json` (accessed as `/mnt/c/Users/<user>/...`)

### JSON structure
```
global:
  folder        - root directory for all git repos
  credential_ssh.enabled / ssh_folder
  credential_gcm.enabled / helper / credentialStore

accounts.<AccountKey>:
  url, username, folder, name, email
  gcm_provider, gcm_useHttpPath   (for GCM)
  ssh_host, ssh_hostname, ssh_type (for SSH)
  repos.<RepoName>:
    credential_type: "gcm" | "ssh"
    name, email     (optional per-repo overrides)
    folder          (optional: absolute path or relative to account folder)
```

## WSL2 specifics

On WSL2, `git-config-repos.sh` uses `git.exe` (Windows Git) instead of Linux `git` to avoid WSL2 filesystem performance issues and to integrate with the Windows Credential Manager. Path conversions between WSL (`/mnt/c/...`) and Windows (`C:\...`) are handled internally via `convert_wsl_to_windows_path()`.

## Credential types

- **GCM (HTTPS):** Uses Git Credential Manager. Credentials are stored in the OS keychain (Windows Credential Manager / macOS Keychain / Linux Secret Service). The script triggers browser-based authentication when credentials are missing.
- **SSH:** Generates per-account ed25519 (or configured type) key pairs at `~/.ssh/<ssh_host>-sshkey`. Adds an `Include` directive to `~/.ssh/config` pointing to `~/.ssh/git-config-repos` which is regenerated on each run.

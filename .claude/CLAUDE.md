# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

## What this repo is

A multi-component project for managing Git multi-account environments across providers (GitHub, GitLab, Gitea, Forgejo). Three sub-projects:

1. **`git-toolkit/`** — Native C++ app (core library + CLI + platform-native GUIs). In development.
2. **`git-config-repos/`** — Bash script for automated repo configuration. Production, power users.
3. **`git-status-pull/`** — Bash script for sync status and auto-pull. Production, power users.

## Repository layout

```text
git-toolkit/              # C++ core + CLI + GUI (SwiftUI/Win32/GTK)
  core/                   # libgittoolkit shared library
  cli/                    # CLI frontend
  gui/{macos,windows,linux}/
  docs/

git-config-repos/         # Shell script sub-project
  git-config-repos.sh
  git-config-repos.jsonc  # Annotated example config
  git-config-repos.schema.json  # v1 JSON Schema
  docs/

git-status-pull/          # Shell script sub-project
  git-status-pull.sh
  docs/
```

## Shell scripts

### `git-config-repos/git-config-repos.sh`

Reads `~/.config/git-config-repos/git-config-repos.json` and for each configured account/repo: verifies credentials, clones repos that don't exist, and fixes configuration of existing ones. Supports GCM, SSH, and token credential types per-repo.

```bash
./git-config-repos/git-config-repos.sh
./git-config-repos/git-config-repos.sh --dry-run
```

### `git-status-pull/git-status-pull.sh`

Scans all `.git` directories from CWD and reports sync status. Can auto-pull if safe.

```bash
./git-status-pull/git-status-pull.sh
./git-status-pull/git-status-pull.sh -v pull
```

## Platform detection

Both scripts share the same detection block: `PLATFORM` (`wsl2` | `gitbash` | `macos` | `linux`) and `cmdgit` (`git.exe` on WSL2, `git` elsewhere).

## C++ core (git-toolkit/)

Built with CMake, C++20. Dependencies: nlohmann/json (header-only), Catch2 or GoogleTest.

Credential stores use native OS APIs: Security.framework (macOS), Win32 CredRead/CredWrite (Windows), libsecret (Linux).

macOS GUI uses SwiftUI with a C bridge layer to the C++ core. Windows GUI uses Win32. Linux GUI uses GTK4 via gtkmm.

## Config format

**v1** (current, shell scripts): `~/.config/git-config-repos/git-config-repos.json`
**v2** (git-toolkit app): `~/.config/git-toolkit/git-toolkit.json` — renames `accounts` to `sources`, real booleans, adds `"token"` credential type.

## Workflow Orchestration

**Priority order when rules conflict**: Correctness > Simplicity > Elegance.

### 1. Plan First

- Enter plan mode for ANY non-trivial task (3+ steps or architectural decisions)
- If something goes sideways, STOP and re-plan immediately
- Write detailed specs upfront to reduce ambiguity

### 2. Subagent Strategy

- Use subagents liberally to keep main context window clean
- Offload research, exploration, and parallel analysis to subagents
- One task per subagent for focused execution

### 3. Verify Before Done

- Never mark a task complete without proving it works
- Test scripts with `bash -n` (syntax check) and `shellcheck` when available
- C++ builds: `cmake --build` + `ctest`
- Validate config templates render correctly before committing

### 4. Autonomous Bug Fixing

- When given a bug report: just fix it
- Point at logs, errors, failing tests — then resolve them

### 5. Learn from Corrections

- After corrections from the user: save a `feedback` memory via the memory system

## Core Principles

- **Simplicity First**: Make every change as simple as possible
- **No Laziness**: Find root causes. No temporary fixes.
- **Minimal Impact**: Changes should only touch what's necessary
- **Skills first**: Check if a skill exists before doing work manually
- **Zero entropy**: Never create files outside defined structure

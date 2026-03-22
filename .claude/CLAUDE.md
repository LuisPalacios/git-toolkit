# CLAUDE.md

This file provides guidance to Claude Code when working with code in this repository.

## What this repo is

A multi-component project for managing Git multi-account environments across providers (GitHub, GitLab, Gitea, Forgejo, Bitbucket).

1. **Go app** (`cmd/cli/`, `cmd/gui/`, `pkg/`) — CLI + Wails GUI sharing a Go library. In development.
2. **`git-config-repos/`** — Bash script for automated repo configuration. Production, power users. UNTOUCHED.
3. **`git-status-pull/`** — Bash script for sync status and auto-pull. Production, power users. UNTOUCHED.

## Repository layout

```text
cmd/
  cli/                    Go CLI binary (git-toolkit)
  gui/                    Wails v2 + Svelte GUI (git-toolkit-gui)
pkg/                      Shared Go library
  config/                 Config v2 model, load/save, v1→v2 migration
  git/                    Git subprocess operations (os/exec)
  provider/               Provider API clients (GitHub, GitLab, Gitea, Forgejo)
  mirror/                 Repo migration + push mirrors
  status/                 Clone status checking
docs/
  user-guide.md           End-user documentation
  developer-guide.md      Build instructions, contributing
  architecture.md         Technical design
  migration.md            v1→v2 migration guide
git-config-repos/         Shell script sub-project (UNTOUCHED)
git-status-pull/          Shell script sub-project (UNTOUCHED)
git-toolkit.schema.json   v2 JSON Schema
git-toolkit.jsonc         v2 annotated example (Spanish comments)
go.mod / go.sum           Go module
README.md                 Project overview
```

## Go app

**Language:** Go. **GUI:** Wails v2 + Svelte. **Two independent binaries** from shared `pkg/`.

```bash
# Build CLI
go build -o git-toolkit ./cmd/cli

# Build GUI (requires Wails CLI)
cd cmd/gui && wails build

# Run tests
go test ./pkg/...
```

Git operations shell out to system `git` via `os/exec`. Provider APIs use `net/http`.

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

## Config format

**v1** (shell scripts): `~/.config/git-config-repos/git-config-repos.json` — `accounts`, string booleans, `gcm`/`ssh` only.

**v2** (Go app): `~/.config/git-toolkit/git-toolkit.json` — `sources`, real booleans, `gcm`/`ssh`/`token`, nested SSH/GCM objects, `provider` field, `version: 2`.

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
- Go: `go build ./...` + `go test ./...`
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

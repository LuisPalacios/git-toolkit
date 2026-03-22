<p align="center">
  <img src="assets/logo.svg" width="128" alt="git-toolkit">
</p>

<h1 align="center">Git-Toolkit</h1>

<p align="center">
  <a href="https://github.com/LuisPalacios/git-toolkit/actions/workflows/release.yaml">
    <img src="https://github.com/LuisPalacios/git-toolkit/actions/workflows/release.yaml/badge.svg" alt="Release" />
  </a>
</p>

<p align="center">
  <strong>One tool for all you clones</strong><br>
  Manage Git repositories across multiple accounts from a single tool
</p>

---

Supports **GitHub**, **GitLab**, **Gitea**, **Forgejo**, **Bitbucket**, **etc** — on Windows, macOS, Linux, and headless servers.

## What It Does

git-toolkit unifies multi-account Git management into two binaries:

- **`git-toolkit`** — CLI for power users and headless servers
- **`git-toolkit-gui`** — Desktop GUI for everyone else (Wails + Svelte)

Both share the same Go library and read the same configuration file.

## Features

- **Account management** — Add and configure accounts for any Git provider
- **Three auth methods** — GCM (browser OAuth), SSH (key pairs), Token (PAT)
- **Repo discovery** — Query provider APIs to find and select repos to manage
- **Clone & pull** — Clone all or selected repos, pull where safe
- **Status dashboard** — See which repos need pull, have uncommitted changes, or are ahead/behind
- **Repo migration** — Migrate repos between providers via `git clone --mirror`
- **Push mirrors** — Set up automated backups to Forgejo/Gitea
- **Guided auth setup** — The GUI walks non-technical users through GCM browser authentication step by step

## Quick Start

```bash
# Install (download from Releases or build from source)
go install github.com/LuisPalacios/git-toolkit/cmd/cli@latest

# Migrate from existing git-config-repos.json (v1)
git-toolkit migrate

# Or start fresh
git-toolkit init

# Check status of all clones
git-toolkit status

# Discover repos on a provider
git-toolkit discover "GitHub-Personal"

# Clone everything
git-toolkit clone --all

# Launch the GUI
git-toolkit-gui
```

## Target Environments

| Environment | Binary | Recommended auth |
| ----------- | ------ | ---------------- |
| Windows / macOS / Linux desktop | CLI + GUI | GCM (browser OAuth) |
| Headless Linux server (SSH) | CLI only | SSH (key pairs) |
| CI / automation | CLI only | Token or SSH |

## Configuration

git-toolkit uses a JSON configuration file at `~/.config/git-toolkit/git-toolkit.json` (v2 format).

See the [annotated example](git-toolkit.jsonc) for a comprehensive template with comments in Spanish, or the [JSON Schema](git-toolkit.schema.json) for editor autocompletion and validation.

Add the schema reference to your config for IDE support:

```json
{
    "$schema": "https://raw.githubusercontent.com/LuisPalacios/git-toolkit/main/git-toolkit.schema.json",
    "version": 2,
    "global": { ... },
    "sources": { ... }
}
```

## Documentation

- [User Guide](docs/user-guide.md) — Installation, setup, features, troubleshooting
- [Developer Guide](docs/developer-guide.md) — Building from source, project structure, contributing
- [Architecture](docs/architecture.md) — Technical design, component diagram, v1 vs v2 format
- [Migration Guide](docs/migration.md) — Migrating from `git-config-repos.sh` (v1 format)

## Background

More context on multi-account Git management: [Git Multicuenta](https://luispa.com/posts/2024-09-21-git-multicuenta/).

## Legacy Scripts

The original Bash scripts remain available and fully functional:

- **[git-config-repos.sh](git-config-repos/docs/README.md)** — Automated repo configuration for power users. Reads v1 format.
- **[git-status-pull.sh](git-status-pull/docs/README.md)** — Repo sync status and auto-pull. Works standalone.

These scripts are independent of git-toolkit and will continue to work with their v1 configuration format.

## Building

```bash
# Prerequisites: Go 1.22+, Node.js 20+, Wails CLI v2

# CLI only
go build -o git-toolkit ./cmd/cli

# GUI (requires Wails)
cd cmd/gui && wails build
```

See the [Developer Guide](docs/developer-guide.md) for detailed build instructions and cross-compilation.

## License

[MIT](LICENSE)

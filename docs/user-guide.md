# User Guide

## Overview

git-toolkit helps you manage Git repositories across multiple accounts and providers (GitHub, GitLab, Gitea, Forgejo, Bitbucket) from a single configuration file and a unified interface.

Two binaries are available:

- **`git-toolkit`** (CLI) — For power users and headless servers. Runs anywhere Git runs.
- **`git-toolkit-gui`** (GUI) — For desktop users who prefer a visual interface. Modern, mouse-driven.

Both read the same configuration file at `~/.config/git-toolkit/git-toolkit.json`.

The configuration has two main sections:

- **`accounts`** — WHO you are on each server (credentials, identity)
- **`sources`** — WHAT you clone from each account (repos organized by org/repo)

---

## Installation

### Windows

Download the latest release from the [Releases page](https://github.com/LuisPalacios/git-toolkit/releases):

- `git-toolkit-windows-amd64.exe` — CLI
- `git-toolkit-gui-windows-amd64.exe` — GUI

Place them in a directory on your `PATH` (e.g., `C:\Users\<you>\bin`).

**Prerequisites:** [Git for Windows](https://gitforwindows.org/) must be installed.

### macOS

```bash
curl -LO https://github.com/LuisPalacios/git-toolkit/releases/latest/download/git-toolkit-darwin-arm64.tar.gz
tar xzf git-toolkit-darwin-arm64.tar.gz
sudo mv git-toolkit git-toolkit-gui /usr/local/bin/
```

**Prerequisites:** Git (via Xcode Command Line Tools or Homebrew).

### Linux

```bash
curl -LO https://github.com/LuisPalacios/git-toolkit/releases/latest/download/git-toolkit-linux-amd64.tar.gz
tar xzf git-toolkit-linux-amd64.tar.gz
sudo mv git-toolkit /usr/local/bin/
# GUI only if you have a desktop environment:
sudo mv git-toolkit-gui /usr/local/bin/
```

**Prerequisites:** Git. For the GUI, WebKitGTK is required (`sudo apt install libwebkit2gtk-4.1-dev` on Debian/Ubuntu).

---

## First-Time Setup

### Option 1: Interactive init (recommended)

```bash
git-toolkit init
```

This creates `~/.config/git-toolkit/git-toolkit.json` with your global settings (root folder, credential store). It auto-detects your OS for the credential store.

Then add your accounts, sources, and repos:

```bash
# Add an account (who you are on a server)
git-toolkit account add my-github \
  --provider github \
  --url https://github.com \
  --username YourUser \
  --name "Your Name" \
  --email "you@example.com" \
  --default-credential-type gcm

# Add a source (what you clone from that account)
git-toolkit source add my-github --account my-github

# Add repos (org/repo format)
git-toolkit repo add my-github "YourUser/my-project"
git-toolkit repo add my-github "YourUser/dotfiles"
```

### Option 2: Migrate from git-config-repos.sh (v1)

If you already have a `~/.config/git-config-repos/git-config-repos.json`:

```bash
# Preview what the migration will produce
git-toolkit migrate --dry-run

# Run the actual migration
git-toolkit migrate
```

The original v1 file is never modified. Both tools can coexist.

### Option 3: Launch the GUI

```bash
git-toolkit-gui
```

The setup wizard will guide you through creating accounts and discovering repos.

---

## Account Management

An account represents WHO you are on a git server — one unique `(hostname, username)` pair.

### Adding accounts

```bash
# GitHub personal
git-toolkit account add github-personal \
  --provider github \
  --url https://github.com \
  --username LuisPalacios \
  --name "Luis Palacios" \
  --email "luis@example.com" \
  --default-credential-type gcm

# Forgejo homelab
git-toolkit account add forgejo-homelab \
  --provider forgejo \
  --url https://forge.mylab.lan \
  --username luis \
  --name "Luis Palacios" \
  --email "luis@mylab.lan" \
  --default-credential-type ssh

# GitLab
git-toolkit account add gitlab-work \
  --provider gitlab \
  --url https://gitlab.com \
  --username youruser \
  --name "Your Name" \
  --email "you@company.com"
```

### Listing and inspecting accounts

```bash
git-toolkit account list
git-toolkit account show github-personal
git-toolkit account show github-personal --json
```

### Updating an account

Only the flags you specify are changed:

```bash
git-toolkit account update github-personal --name "New Name" --email "new@email.com"
```

### Deleting an account

An account can only be deleted if no sources reference it:

```bash
git-toolkit account delete github-personal
# Error: cannot delete — referenced by source "github-personal"

git-toolkit source delete github-personal
git-toolkit account delete github-personal  # now succeeds
```

---

## Source Management

A source represents WHAT you clone from an account. Each source references one account and contains a list of repos.

### Adding a source

```bash
git-toolkit source add github-personal --account github-personal
git-toolkit source add forgejo-homelab --account forgejo-homelab
```

By default, the source key is used as the first-level clone folder. To override:

```bash
git-toolkit source add my-server --account forgejo-homelab --folder "server-repos"
```

### Listing sources

```bash
git-toolkit source list
git-toolkit source list --account github-personal
```

---

## Repo Management

Repos use `org/repo` format. The org part becomes the second-level folder, the repo part becomes the third-level (clone) folder.

### Adding repos

```bash
# Simple — inherits credential type from account default
git-toolkit repo add github-personal "LuisPalacios/git-toolkit"
git-toolkit repo add github-personal "LuisPalacios/dotfiles"

# Cross-org — access another org's repo with your credentials
git-toolkit repo add github-personal "other-org/their-repo"

# Multiple orgs on same server (Forgejo/Gitea)
git-toolkit repo add forgejo-homelab "infra/homelab-ops"
git-toolkit repo add forgejo-homelab "infra/migra-forgejo"
git-toolkit repo add forgejo-homelab "familia/ines-denia"

# Override credential type for a specific repo
git-toolkit repo add github-personal "LuisPalacios/private-repo" --credential-type ssh
```

### Folder overrides

```bash
# Override the 2nd level folder (org → custom name)
git-toolkit repo add github-work "Sumwall/sumwall.github.io" --id-folder "sumwall-rest"
# Clones to: ~/git/github-work/sumwall-rest/sumwall.github.io/

# Override the 3rd level folder (clone name)
git-toolkit repo add github-work "Sumwall/sumwall.web" --clone-folder "website"
# Clones to: ~/git/github-work/Sumwall/website/

# Absolute path — replaces everything
git-toolkit repo add forgejo-homelab "luis/my-config" --clone-folder "~/.config/my-config"
# Clones to: ~/.config/my-config/
```

### Listing and inspecting repos

```bash
git-toolkit repo list
git-toolkit repo list --source github-personal
git-toolkit repo show github-personal "LuisPalacios/git-toolkit"
```

### Updating a repo

```bash
git-toolkit repo update github-personal "LuisPalacios/private-repo" --credential-type gcm
git-toolkit repo update forgejo-homelab "infra/homelab-ops" --id-folder "infra-prod"
```

### Deleting a repo

```bash
git-toolkit repo delete github-personal "LuisPalacios/old-project"
```

---

## Folder Structure

Repos are cloned into a three-level directory structure:

```text
~/git/                              ← global.folder
  <source-key>/                     ← 1st level (or source.folder if set)
    <org>/                          ← 2nd level (from repo key, or id_folder override)
      <repo>/                      ← 3rd level (from repo key, or clone_folder override)
```

Example with real data:

```text
~/00.git/
  git-parchis/                      ← source key
    familia/ines-denia/             ← org/repo
    infra/homelab-ops/
    infra/migra-forgejo/
  github-LuisPalacios/             ← source key
    LuisPalacios/git-toolkit/       ← org/repo
    kollective-networks/kltv.kombine/  ← cross-org
  github-sumwall/
    Sumwall/sumwall.browser/
    sumwall-rest/sumwall.github.io/ ← id_folder override
```

---

## Authentication Setup

git-toolkit supports three authentication methods. Choose based on your environment:

| Method | Best for | How it works |
| ------ | -------- | ------------ |
| **GCM** | Desktops (Windows, macOS, Linux with GUI) | Browser-based OAuth, credentials in OS keystore |
| **SSH** | Headless servers, advanced users | SSH key pairs with host aliases |
| **Token** | CI/CD, automation, Gitea/Forgejo | Personal Access Token in OS keystore |

Set the default per account:

```bash
git-toolkit account add my-server \
  --provider forgejo \
  --url https://forge.lan \
  --username luis \
  --name "Luis" \
  --email "luis@lan" \
  --default-credential-type ssh    # all repos use SSH unless overridden
```

Override per repo:

```bash
git-toolkit repo add my-server "infra/special" --credential-type token
```

### Credential store by platform

- **Windows:** `wincredman` (Windows Credential Manager)
- **macOS:** `keychain` (Keychain Access)
- **Linux:** `secretservice` (GNOME Keyring / KWallet)

---

## Status Monitoring

```bash
# All repos
git-toolkit status

# Filter by source
git-toolkit status --source github-personal

# JSON output (for scripting)
git-toolkit status --json
```

Status indicators:

| Symbol | State | Meaning |
| ------ | ----- | ------- |
| ✓ | clean | Up to date |
| ✗ | dirty | Uncommitted changes |
| ↓ | behind | Needs pull |
| ↑ | ahead | Needs push |
| ⚠ | diverged | Both ahead and behind |
| ⚡ | conflict | Merge conflicts |
| ○ | not cloned | Directory doesn't exist |
| ~ | no upstream | No tracking branch |

---

## Cloning

```bash
# Clone all repos from all sources
git-toolkit clone --all

# Clone repos for a specific source
git-toolkit clone --source github-personal

# Clone a specific repo
git-toolkit clone --source github-personal --repo "LuisPalacios/git-toolkit"
```

---

## Pulling

```bash
# Pull all repos that are behind (fast-forward only)
git-toolkit pull --all

# Pull for a specific source
git-toolkit pull --source github-personal
```

Dirty or conflicted repos are skipped with a warning.

---

## Migration from v1

```bash
# Preview
git-toolkit migrate --dry-run

# Execute
git-toolkit migrate
```

See [migration.md](migration.md) for details on what changes and how accounts are deduplicated.

---

## Configuration File Reference

The config lives at `~/.config/git-toolkit/git-toolkit.json`. See [git-toolkit.jsonc](../git-toolkit.jsonc) for a fully annotated example.

### Global

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `folder` | string | Yes | Root directory for all clones. Supports `~`. |
| `credential_ssh.enabled` | boolean | No | Enable SSH credential management. |
| `credential_ssh.ssh_folder` | string | No | SSH config directory. Default `~/.ssh`. |
| `credential_gcm.enabled` | boolean | No | Enable GCM authentication. |
| `credential_gcm.helper` | string | No | Credential helper. Typically `"manager"`. |
| `credential_gcm.credential_store` | string | No | `"wincredman"`, `"keychain"`, or `"secretservice"`. |

### Account

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `provider` | string | Yes | `"github"`, `"gitlab"`, `"gitea"`, `"forgejo"`, `"bitbucket"`, `"generic"` |
| `url` | string | Yes | Server URL (scheme+host, no path). |
| `username` | string | Yes | Account username. |
| `name` | string | Yes | Default `git user.name`. |
| `email` | string | Yes | Default `git user.email`. |
| `default_branch` | string | No | Default branch (e.g., `"main"`). |
| `default_credential_type` | string | No | Default auth: `"gcm"`, `"ssh"`, or `"token"`. |
| `ssh.host` | string | No | SSH Host alias. |
| `ssh.hostname` | string | No | Real SSH hostname. |
| `ssh.key_type` | string | No | `"ed25519"` or `"rsa"`. |
| `gcm.provider` | string | No | GCM provider hint. |
| `gcm.use_http_path` | boolean | No | Scope credentials by HTTP path. |

### Source

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `account` | string | Yes | References an account key. |
| `folder` | string | No | Override first-level clone folder. Default: source key. |

### Repo (within source.repos)

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `credential_type` | string | No | Override auth method. Inherits from account. |
| `name` | string | No | Override `git user.name`. |
| `email` | string | No | Override `git user.email`. |
| `id_folder` | string | No | Override 2nd level dir (org folder). |
| `clone_folder` | string | No | Override 3rd level dir. If absolute, replaces entire path. |

---

## Troubleshooting

### Config not found

```bash
# Check where git-toolkit looks for the config
git-toolkit config path

# Create a new config
git-toolkit init

# Or specify a custom path
git-toolkit status --config /path/to/my-config.json
```

### GCM opens the wrong browser account

Clear cached credentials:

- **Windows:** Control Panel > Credential Manager > remove `git:https://github.com` entries
- **macOS:** Keychain Access > search `github.com` > delete
- **Linux:** `secret-tool clear protocol https host github.com`

### SSH connection refused

```bash
ssh -T git@gh-YourAlias -v
# Check: key added to provider, correct IdentityFile, ssh-agent running
```

### "Repository not found" on clone

- Verify the `org/repo` name matches the actual repo
- Verify your credentials: test with `git ls-remote <url>`
- For cross-org repos, make sure your account has access

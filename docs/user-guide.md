# User Guide

## Overview

git-toolkit helps you manage Git repositories across multiple accounts and providers (GitHub, GitLab, Gitea, Forgejo, Bitbucket) from a single configuration file and a unified interface.

Two binaries are available:

- **`git-toolkit`** (CLI) — For power users and headless servers. Runs anywhere Git runs.
- **`git-toolkit-gui`** (GUI) — For desktop users who prefer a visual interface. Modern, mouse-driven.

Both read the same configuration file at `~/.config/git-toolkit/git-toolkit.json`.

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
# Download and extract (example for arm64)
curl -LO https://github.com/LuisPalacios/git-toolkit/releases/latest/download/git-toolkit-darwin-arm64.tar.gz
tar xzf git-toolkit-darwin-arm64.tar.gz
sudo mv git-toolkit git-toolkit-gui /usr/local/bin/
```

**Prerequisites:** Git (via Xcode Command Line Tools or Homebrew).

### Linux

```bash
# Download and extract (example for amd64)
curl -LO https://github.com/LuisPalacios/git-toolkit/releases/latest/download/git-toolkit-linux-amd64.tar.gz
tar xzf git-toolkit-linux-amd64.tar.gz
sudo mv git-toolkit /usr/local/bin/
# GUI only if you have a desktop environment:
sudo mv git-toolkit-gui /usr/local/bin/
```

**Prerequisites:** Git. For the GUI, WebKitGTK is required (`sudo apt install libwebkit2gtk-4.1-dev` on Debian/Ubuntu).

---

## First-Time Setup

### Using the GUI

1. Launch `git-toolkit-gui`
2. The setup wizard will appear since no configuration file exists yet
3. Follow the steps:
   - Set your root folder for Git clones (e.g., `~/00.git`)
   - Add your first Git provider account
   - Choose your authentication method (GCM recommended for desktops)
   - Authenticate via the guided flow
   - Discover and select repositories to manage
4. Your configuration is saved to `~/.config/git-toolkit/git-toolkit.json`

### Using the CLI

```bash
# Create a minimal configuration interactively
git-toolkit init

# Or migrate from an existing git-config-repos.json (v1 format)
git-toolkit migrate
```

---

## Account Configuration

### Adding a GitHub Account

**GUI:** Click "Add Source" > Select "GitHub" > Enter your username and URL.

**CLI:**

```bash
git-toolkit config add-source \
  --name "GitHub-Personal" \
  --provider github \
  --url "https://github.com/YourUsername" \
  --username "YourUsername" \
  --folder "github-personal" \
  --user-name "Your Name" \
  --user-email "you@example.com"
```

### Adding a Forgejo/Gitea Homelab

```bash
git-toolkit config add-source \
  --name "Forgejo-Homelab" \
  --provider forgejo \
  --url "https://forge.mylab.lan/myuser" \
  --username "myuser" \
  --folder "forgejo-homelab" \
  --user-name "Your Name" \
  --user-email "you@mylab.lan"
```

### Adding a GitLab Account

```bash
git-toolkit config add-source \
  --name "GitLab-Work" \
  --provider gitlab \
  --url "https://gitlab.com/youruser" \
  --username "youruser" \
  --folder "gitlab-work" \
  --user-name "Your Name" \
  --user-email "you@company.com"
```

---

## Authentication Setup

git-toolkit supports three authentication methods. Choose based on your environment:

| Method | Best for | How it works |
| ------ | -------- | ------------ |
| **GCM** | Desktops (Windows, macOS, Linux with GUI) | Browser-based OAuth, credentials stored in OS keystore |
| **SSH** | Headless servers, advanced users | SSH key pairs with host aliases in `~/.ssh/config` |
| **Token** | CI/CD, automation, Gitea/Forgejo | Personal Access Token stored in OS keystore |

### GCM (Git Credential Manager) — Recommended for Desktops

GCM handles authentication through your browser and stores credentials securely in your operating system's native keystore.

**GUI guided flow:**

1. Select a source and click "Setup Authentication" > "GCM"
2. The wizard will display: *"Prepare your browser — you'll be redirected to authenticate"*
3. A browser window opens to your provider's OAuth page
4. **Important for multiple accounts:** If you have multiple accounts on the same provider (e.g., personal + work GitHub), make sure you're logged into the correct account in your browser before granting access
5. Grant permissions for git-toolkit
6. The wizard confirms: *"Credentials stored securely in [Windows Credential Manager / Keychain / Secret Service]"*
7. Repeat for each account that uses GCM

**CLI setup:**

```bash
# GCM is configured automatically when you clone the first repo
git-toolkit clone --source "GitHub-Personal"
# The browser will open for authentication on the first clone
```

**Credential store by platform:**

- **Windows:** Windows Credential Manager (`wincredman`)
- **macOS:** Keychain Access (`keychain`)
- **Linux:** GNOME Keyring or KWallet via Secret Service (`secretservice`)

### SSH — Recommended for Headless Servers

SSH uses key pairs. git-toolkit can generate keys and configure `~/.ssh/config` automatically.

**Setup:**

```bash
# Generate SSH key and configure ~/.ssh/config
git-toolkit auth setup-ssh --source "GitHub-Personal"
# Output:
#   Generated key: ~/.ssh/id_ed25519_git_gh-LuisPalacios
#   Added host alias to ~/.ssh/config:
#     Host gh-LuisPalacios
#       HostName github.com
#       User git
#       IdentityFile ~/.ssh/id_ed25519_git_gh-LuisPalacios
#       IdentitiesOnly yes
#
#   Public key (add this to GitHub > Settings > SSH Keys):
#   ssh-ed25519 AAAA... luis@example.com

# Test the connection
git-toolkit auth test --source "GitHub-Personal"
```

Repos configured with `credential_type: "ssh"` will clone using the host alias:
`git@gh-LuisPalacios:LuisPalacios/repo.git`

### Token (PAT) — For Automation or When GCM/SSH Aren't Available

Tokens are stored in the OS credential store, **never** in the configuration file.

```bash
# Store a token securely
git-toolkit auth set-token --source "Gitea-Server"
# You'll be prompted to paste your Personal Access Token
# It's stored in the OS keystore, not in git-toolkit.json
```

> **Security warning:** Tokens can leak if accidentally committed to a repository or left in shell history. Prefer SSH or GCM when possible.

---

## Repo Discovery and Cloning

### Discovering Repos

Query the provider's API to see all available repositories:

```bash
# List all repos for a source
git-toolkit discover "GitHub-Personal"

# Output:
#   Found 47 repositories for GitHub-Personal:
#   [tracked]  git-toolkit
#   [tracked]  dotfiles
#   [new]      my-new-project
#   [new]      another-repo
#   ...
```

**GUI:** Click the "Discover" button on any source. A panel shows all remote repos with checkboxes to select which ones to track.

### Adding Discovered Repos

```bash
# Add specific repos to track
git-toolkit config add-repo --source "GitHub-Personal" --repo "my-new-project" --credential-type gcm

# Add all untracked repos at once
git-toolkit config add-repo --source "GitHub-Personal" --all --credential-type gcm
```

### Cloning

```bash
# Clone all repos for all sources
git-toolkit clone --all

# Clone repos for a specific source
git-toolkit clone --source "GitHub-Personal"

# Clone a specific repo
git-toolkit clone --source "GitHub-Personal" --repo "my-new-project"
```

---

## Status Monitoring

Check the sync status of your clones:

```bash
# Show status of all repos
git-toolkit status

# Output:
#   GitHub-Personal/git-toolkit        ✓ clean
#   GitHub-Personal/dotfiles           ↓ 3 behind
#   Forgejo-Homelab/infra-docker       ✗ dirty (2 modified)
#   Forgejo-Homelab/home-automation    ↑ 1 ahead
#   GitLab-Work/pipeline-templates     ⚠ diverged (2 ahead, 1 behind)

# Filter by source
git-toolkit status --source "Forgejo-Homelab"

# JSON output for scripting
git-toolkit status --json
```

**GUI:** The main dashboard shows all repos with color-coded status indicators. Click "Refresh Status" for an on-demand update — it runs asynchronously so you can keep working. Status auto-refreshes in the background while the app is open.

### Pulling

```bash
# Pull all repos that are behind (fast-forward only)
git-toolkit pull --all

# Pull for a specific source
git-toolkit pull --source "GitHub-Personal"

# Pull a specific repo
git-toolkit pull --source "GitHub-Personal" --repo "git-toolkit"
```

---

## Repo Migration Between Providers

Migrate repositories from one provider to another using `git clone --mirror`:

```bash
# Migrate a repo from GitHub to Forgejo homelab
git-toolkit migrate-repo \
  --from "GitHub-Personal" \
  --to "Forgejo-Homelab" \
  --repo "my-private-project"

# This will:
#   1. Create a bare mirror clone of the repo
#   2. Create the repo on the target provider via API
#   3. Push the mirror to the target
#   4. Optionally add the repo to the target source in your config
```

**GUI:** Use the Migration Wizard — select source, target, and repos to migrate.

---

## Push Mirrors (Automated Backups)

Set up automated push mirrors from one provider to another (requires Gitea/Forgejo as target):

```bash
# Set up a push mirror from GitHub to Forgejo
git-toolkit mirror setup \
  --source "GitHub-Personal" \
  --target "Forgejo-Homelab" \
  --repo "important-project"
```

This uses the Forgejo/Gitea mirror API to set up automatic syncing.

---

## Configuration File Reference

The configuration lives at `~/.config/git-toolkit/git-toolkit.json`. See the annotated example at [git-toolkit.jsonc](../git-toolkit.jsonc) for a complete reference with comments.

### Global Section

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `folder` | string | Yes | Root directory for all clones. Supports `~` prefix. |
| `credential_ssh.enabled` | boolean | No | Enable SSH credential management. |
| `credential_ssh.ssh_folder` | string | No | SSH config directory. Default `~/.ssh`. |
| `credential_gcm.enabled` | boolean | No | Enable GCM (HTTPS) authentication. |
| `credential_gcm.helper` | string | No | Credential helper. Typically `"manager"`. |
| `credential_gcm.credential_store` | string | No | `"wincredman"`, `"keychain"`, or `"secretservice"`. |

### Source Section

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `provider` | string | Yes | `"github"`, `"gitlab"`, `"gitea"`, `"forgejo"`, `"bitbucket"`, `"generic"` |
| `url` | string (URI) | Yes | Provider base URL with optional user/org path. |
| `username` | string | Yes | Account username on the provider. |
| `folder` | string | Yes | Subdirectory under `global.folder`. |
| `name` | string | Yes | Default `git user.name` for repos. |
| `email` | string (email) | Yes | Default `git user.email` for repos. |
| `default_branch` | string | No | Default branch name (e.g., `"main"`). |
| `ssh.host` | string | No | SSH Host alias for `~/.ssh/config`. |
| `ssh.hostname` | string | No | Real SSH hostname. |
| `ssh.key_type` | string | No | `"ed25519"` (recommended) or `"rsa"`. |
| `gcm.provider` | string | No | GCM provider hint: `"github"`, `"gitlab"`, `"bitbucket"`, `"generic"`. |
| `gcm.use_http_path` | boolean | No | Scope credentials by HTTP path. |

### Repo Section

| Field | Type | Required | Description |
| ----- | ---- | -------- | ----------- |
| `credential_type` | string | Yes | `"gcm"`, `"ssh"`, or `"token"`. |
| `name` | string | No | Override `git user.name`. |
| `email` | string (email) | No | Override `git user.email`. |
| `folder` | string | No | Custom clone path (absolute or relative). |

---

## Troubleshooting

### GCM opens the wrong browser account

Make sure you're logged into the correct account in your browser before GCM triggers the OAuth flow. If credentials are cached for the wrong account, clear them:

- **Windows:** Control Panel > Credential Manager > Windows Credentials > remove entries starting with `git:https://github.com`
- **macOS:** Keychain Access > search for `github.com` > delete the entry
- **Linux:** `secret-tool clear protocol https host github.com`

### SSH connection refused

```bash
# Test the connection
ssh -T git@gh-YourAlias -v

# Common issues:
# - Key not added to the provider's SSH settings
# - Wrong IdentityFile path in ~/.ssh/config
# - SSH agent not running: eval "$(ssh-agent -s)" && ssh-add ~/.ssh/id_ed25519_git_...
```

### "Repository not found" on clone

- Verify the URL in your config matches the actual repo URL
- For cross-org repos, the `url` should point to the host only (e.g., `https://github.com`), and the repo name should include the org (e.g., `"other-org/repo-name"`)
- Verify your credentials are valid: `git-toolkit auth test --source "SourceName"`

### Migrating from git-config-repos.sh (v1)

```bash
git-toolkit migrate
# Reads ~/.config/git-config-repos/git-config-repos.json (v1)
# Writes ~/.config/git-toolkit/git-toolkit.json (v2)
# Original v1 file is NOT modified
```

See [migration.md](migration.md) for details.

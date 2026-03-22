# Migration Guide

## From git-config-repos.sh (v1) to git-toolkit (v2)

### Overview

git-toolkit uses a new configuration format (v2) stored at a different path. The migration is **non-destructive** — your original v1 config file is never modified.

| | v1 | v2 |
| - | -- | -- |
| **Tool** | `git-config-repos.sh` | `git-toolkit` (CLI) / `git-toolkit-gui` (GUI) |
| **Config file** | `~/.config/git-config-repos/git-config-repos.json` | `~/.config/git-toolkit/git-toolkit.json` |
| **Schema** | `git-config-repos.schema.json` | `git-toolkit.schema.json` |

### Running the Migration

```bash
# Automatic migration
git-toolkit migrate

# What happens:
#   1. Reads ~/.config/git-config-repos/git-config-repos.json
#   2. Converts to v2 format
#   3. Writes ~/.config/git-toolkit/git-toolkit.json
#   4. Original file is NOT modified
```

### What Changes

| v1 | v2 | Example |
| -- | -- | ------- |
| `"accounts": { ... }` | `"sources": { ... }` | Key rename |
| `"enabled": "true"` | `"enabled": true` | String → boolean |
| `"enabled": "false"` | `"enabled": false` | String → boolean |
| `"gcm_useHttpPath": "true"` | `"gcm": { "use_http_path": true }` | Flat → nested + boolean |
| `"credentialStore": "wincredman"` | `"credential_store": "wincredman"` | camelCase → snake_case |
| `"ssh_host": "gh-User"` | `"ssh": { "host": "gh-User" }` | Flat → nested |
| `"ssh_hostname": "github.com"` | `"ssh": { "hostname": "github.com" }` | Flat → nested |
| `"ssh_type": "ed25519"` | `"ssh": { "key_type": "ed25519" }` | Flat → nested + rename |
| `"gcm_provider": "github"` | `"gcm": { "provider": "github" }` | Flat → nested |
| (no version field) | `"version": 2` | Added |
| (no provider field) | `"provider": "github"` | Inferred from `gcm_provider` or set to `"generic"` |
| (no default_branch) | `"default_branch": "main"` | Added with default |

### Organization Merging

When multiple v1 accounts point to the **same server** with the **same username**, they are automatically merged into a single v2 source. This is common for Gitea/Forgejo servers where one user belongs to multiple organizations.

**Before (v1 — 3 separate accounts):**

```json
{
    "accounts": {
        "git-parchis-familia": { "url": "https://git.parchis.org/familia", "username": "luis", ... },
        "git-parchis-infra":   { "url": "https://git.parchis.org/infra",   "username": "luis", ... },
        "git-parchis-luis":    { "url": "https://git.parchis.org/luis",    "username": "luis", ... }
    }
}
```

**After (v2 — 1 merged source):**

```json
{
    "sources": {
        "git-parchis-org": {
            "url": "https://git.parchis.org",
            "username": "luis",
            "folder": "git-parchis",
            "repos": {
                "familia/ines-denia": { "credential_type": "gcm" },
                "infra/homelab-ops": { "credential_type": "gcm" },
                "luis/config-json": { "credential_type": "gcm" }
            }
        }
    }
}
```

**What happens:**
- The org is extracted from each v1 account's URL path (e.g., `/familia` → `familia`)
- Repos get prefixed with `org/` (e.g., `ines-denia` → `familia/ines-denia`)
- The merged source URL points to the server root (no org path)
- The folder is derived from the common prefix of the original folders
- Credentials, name, email are taken from the first account alphabetically
- Per-repo folder overrides are preserved

**When accounts are NOT merged:**
- Different server hostnames → separate sources
- Different usernames on the same server → separate sources

### What Stays the Same

- Repository `credential_type` values
- Per-repo overrides (name, email, folder)
- Global folder path
- Global SSH and GCM settings (just restructured)

### Side-by-Side Usage

Both tools can coexist indefinitely:

- `git-config-repos.sh` continues reading `~/.config/git-config-repos/git-config-repos.json` (v1)
- `git-toolkit` reads `~/.config/git-toolkit/git-toolkit.json` (v2)

**Important:** Changes made in one tool are NOT automatically reflected in the other. If you add a new repo via `git-toolkit`, you'll need to manually add it to the v1 config too (or stop using `git-config-repos.sh`).

### Recommended Migration Path

1. **Run `git-toolkit migrate`** to create the v2 config
2. **Verify the result:** `git-toolkit config show` and compare with your v1 config
3. **Test basic operations:** `git-toolkit status` should show the same repos as `git-status-pull.sh`
4. **Use both tools in parallel** for a while to build confidence
5. **Switch fully to git-toolkit** once you're comfortable
6. **Keep the v1 config** as a backup — it doesn't hurt anything

### Manual Migration

If you prefer to migrate manually, the v2 format is documented in the [annotated example](../git-toolkit.jsonc). The key changes are:

1. Rename `"accounts"` to `"sources"`
2. Add `"version": 2` at the root
3. Add `"provider": "..."` to each source
4. Change string booleans to real booleans
5. Group SSH fields into `"ssh": { ... }` objects
6. Group GCM fields into `"gcm": { ... }` objects
7. Rename `credentialStore` to `credential_store`
8. Save to `~/.config/git-toolkit/git-toolkit.json`

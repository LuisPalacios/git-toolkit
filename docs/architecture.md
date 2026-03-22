# Architecture

## Overview

git-toolkit is a Go monorepo that produces two independent binaries from a shared library:

```text
                 ┌─────────────┐    ┌─────────────────┐
                 │  git-toolkit │    │ git-toolkit-gui  │
                 │    (CLI)     │    │   (Wails app)    │
                 └──────┬──────┘    └────────┬─────────┘
                        │                    │
                        │   ┌────────────────┘
                        │   │
                 ┌──────▼───▼──────┐
                 │     pkg/        │
                 │ (shared Go lib) │
                 ├─────────────────┤
                 │ config/         │ Config v2 model, load/save, migration
                 │ git/            │ Git subprocess operations
                 │ provider/       │ Provider API clients
                 │ mirror/         │ Repo migration + push mirrors
                 │ status/         │ Clone status checking
                 └────────┬───────┘
                          │
              ┌───────────┼───────────┐
              ▼           ▼           ▼
         system git   Provider   OS credential
                       APIs        store
              │           │           │
              ▼           ▼           ▼
         local repos   remote     keychain /
                       repos     wincredman /
                                secretservice
```

---

## Component Details

### pkg/config

Handles the configuration file at `~/.config/git-toolkit/git-toolkit.json`.

```go
type Config struct {
    Schema  string            `json:"$schema,omitempty"`
    Version int               `json:"version"`
    Global  GlobalConfig      `json:"global"`
    Sources map[string]Source  `json:"sources"`
}

type Source struct {
    Provider      string            `json:"provider"`
    URL           string            `json:"url"`
    Username      string            `json:"username"`
    Folder        string            `json:"folder"`
    Name          string            `json:"name"`
    Email         string            `json:"email"`
    DefaultBranch string            `json:"default_branch,omitempty"`
    SSH           *SSHConfig        `json:"ssh,omitempty"`
    GCM           *GCMConfig        `json:"gcm,omitempty"`
    Repos         map[string]Repo   `json:"repos"`
}
```

**v1 compatibility:** `LoadConfig()` detects the format version by checking for `"version": 2` (v2) vs `"accounts"` key (v1). The v1 loader transparently converts to v2 structs in memory.

**Migration:** `Migrate()` reads a v1 file and writes a v2 file, converting:
- `accounts` → `sources`
- `"true"/"false"` → `true/false`
- Flat SSH/GCM fields → nested objects
- `credentialStore` → `credential_store`

### pkg/git

Thin wrapper around `os/exec` for Git operations:

```go
func Clone(url, dest string, opts CloneOpts) error
func Fetch(repoPath string) error
func Pull(repoPath string) error
func Status(repoPath string) (RepoStatus, error)
func MirrorClone(url, dest string) error
func MirrorPush(repoPath, remoteURL string) error
```

All functions shell out to the system `git` binary. No libgit2 dependency.

**Platform detection:** Uses `git` on all platforms. On WSL2, may use `git.exe` when accessing Windows paths (detected at runtime).

### pkg/provider

Abstraction layer for Git hosting provider APIs:

```go
type Provider interface {
    ListRepos() ([]RepoInfo, error)
    CreateRepo(name string, private bool) error
    DeleteRepo(name string) error
    SetupMirror(repo string, targetURL string, token string) error
    TestConnection() error
}
```

**Implementations:**

| Provider | API | Auth | Mirror support |
| -------- | --- | ---- | -------------- |
| GitHub | REST v3 (`api.github.com`) | Token, OAuth device flow | No native API (use git mirror) |
| GitLab | REST v4 (`gitlab.com/api/v4`) | Token | Push mirrors via API |
| Gitea | REST (`/api/v1`) | Token | Push mirrors via API |
| Forgejo | REST (Gitea-compatible `/api/v1`) | Token | Push mirrors via API |
| Bitbucket | REST v2 (`api.bitbucket.org/2.0`) | Token | No native API |
| Generic | None | N/A | Manual only |

**Forgejo note:** Forgejo's API is Gitea-compatible. The `gitea.go` client is reused for both, with the `provider` field used only for display/labeling purposes.

### pkg/status

Checks the sync status of local clones:

```go
type RepoStatus struct {
    Source     string
    Repo       string
    Path       string
    State      State  // Clean, Dirty, Behind, Ahead, Diverged, Conflict, NotCloned
    Ahead      int
    Behind     int
    Modified   int
    Untracked  int
}
```

Runs `git status --porcelain -b` and `git rev-list --count --left-right HEAD...@{upstream}` to determine the state.

### pkg/mirror

Handles repo migration and push mirror setup:

- **Migration:** `git clone --mirror` source → `git push --mirror` target
- **Push mirrors:** Uses Gitea/Forgejo API to configure automatic push mirrors on the server side

---

## Config v1 vs v2

| Aspect | v1 (`git-config-repos.json`) | v2 (`git-toolkit.json`) |
| ------ | ---------------------------- | ----------------------- |
| Top-level key | `accounts` | `sources` |
| Booleans | `"true"` / `"false"` (strings) | `true` / `false` (native) |
| Credential types | `gcm`, `ssh` | `gcm`, `ssh`, `token` |
| SSH config | Flat: `ssh_host`, `ssh_hostname`, `ssh_type` | Nested: `ssh: { host, hostname, key_type }` |
| GCM config | Flat: `gcm_provider`, `gcm_useHttpPath` | Nested: `gcm: { provider, use_http_path }` |
| Credential store | `credentialStore` (camelCase) | `credential_store` (snake_case) |
| Version field | None | `"version": 2` |
| Provider field | None (inferred from `gcm_provider`) | `"provider": "github"` etc. |
| Default branch | None | `"default_branch": "main"` |
| File location | `~/.config/git-config-repos/git-config-repos.json` | `~/.config/git-toolkit/git-toolkit.json` |

Both formats can coexist. The v1 scripts (`git-config-repos.sh`, `git-status-pull.sh`) continue reading v1. git-toolkit reads v2 (and can load v1 for migration).

---

## GUI Architecture (Wails)

```text
┌────────────────────────────────────────────┐
│              Wails Runtime                 │
├────────────────────┬───────────────────────┤
│   Go Backend       │   Svelte Frontend     │
│                    │                       │
│   app.go           │   App.svelte          │
│   ├─ GetConfig()   │   ├─ Sidebar          │
│   ├─ SaveConfig()  │   ├─ RepoGrid         │
│   ├─ Discover()    │   ├─ AccountForm      │
│   ├─ Clone()       │   ├─ DiscoveryPanel   │
│   ├─ GetStatus()   │   ├─ StatusBar        │
│   ├─ Pull()        │   └─ Wizards          │
│   ├─ Migrate()     │                       │
│   └─ SetupAuth()   │                       │
│                    │                       │
│   imports pkg/*    │   calls Go via        │
│                    │   wails.Call()         │
└────────────────────┴───────────────────────┘
```

**Wails bindings:** Go methods on the `App` struct are automatically exposed to the Svelte frontend as TypeScript functions. Wails generates TypeScript types from Go structs.

**Async operations:** Long-running operations (clone, fetch, status refresh) run in goroutines. Progress and results are pushed to the frontend via Wails events:

```go
// Go backend
func (a *App) RefreshStatus() {
    go func() {
        for _, result := range status.CheckAll(a.config) {
            runtime.EventsEmit(a.ctx, "status:update", result)
        }
        runtime.EventsEmit(a.ctx, "status:done")
    }()
}
```

```svelte
<!-- Svelte frontend -->
<script>
  import { EventsOn } from '../wailsjs/runtime'

  EventsOn('status:update', (result) => {
    statuses = [...statuses, result]
  })
</script>
```

---

## Security Considerations

- **Tokens are never stored in the JSON config file.** They go in the OS credential store (Windows Credential Manager, macOS Keychain, Linux Secret Service).
- **The config file contains no secrets** — only URLs, usernames, folder paths, and preference flags.
- **Provider API calls use tokens from the credential store** at runtime, never from config.
- **SSH private keys** are standard `~/.ssh/` files with appropriate permissions (600).

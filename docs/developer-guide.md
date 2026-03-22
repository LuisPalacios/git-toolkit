# Developer Guide

## Prerequisites

- **Go** 1.22+ ([install](https://go.dev/doc/install))
- **Node.js** 20+ ([install](https://nodejs.org/))
- **Git** 2.39+
- **Wails CLI** v2 (for GUI development):
  ```bash
  go install github.com/wailsapp/wails/v2/cmd/wails@latest
  ```

### Platform-Specific

- **Windows:** Git for Windows (provides Git Bash)
- **macOS:** Xcode Command Line Tools
- **Linux:** `libwebkit2gtk-4.1-dev` and `libgtk-3-dev` (for GUI builds)

---

## Building from Source

### CLI Only

```bash
# From the repository root
go build -o git-toolkit ./cmd/cli

# Cross-compile for other platforms
GOOS=linux GOARCH=amd64 go build -o git-toolkit-linux-amd64 ./cmd/cli
GOOS=darwin GOARCH=arm64 go build -o git-toolkit-darwin-arm64 ./cmd/cli
GOOS=windows GOARCH=amd64 go build -o git-toolkit.exe ./cmd/cli
```

### GUI (Wails)

```bash
# Development mode (hot reload)
cd cmd/gui
wails dev

# Production build
wails build
# Output: cmd/gui/build/bin/git-toolkit-gui[.exe]
```

---

## Project Structure

```text
git-toolkit/                    (repo root)
├── cmd/
│   ├── cli/                    CLI binary entry point
│   │   └── main.go             Subcommand registration, flag parsing
│   └── gui/                    Wails GUI app
│       ├── main.go             Wails app initialization
│       ├── app.go              Go methods exposed to Svelte via Wails bindings
│       └── frontend/           Svelte SPA
│           ├── src/
│           │   ├── App.svelte
│           │   ├── lib/        Reusable Svelte components
│           │   └── routes/     Page-level components
│           ├── package.json
│           └── vite.config.js
├── pkg/                        Shared Go library (used by BOTH cli and gui)
│   ├── config/                 Config v2 model, load/save, v1→v2 migration
│   │   ├── config.go           Struct definitions
│   │   ├── load.go             JSON parsing (v1 and v2)
│   │   ├── save.go             JSON serialization
│   │   ├── migrate.go          v1 → v2 conversion
│   │   └── config_test.go
│   ├── git/                    Git subprocess operations
│   │   ├── git.go              Clone, fetch, pull, status
│   │   └── git_test.go
│   ├── provider/               Provider API clients
│   │   ├── provider.go         Interface definition
│   │   ├── github.go           GitHub REST API v3
│   │   ├── gitlab.go           GitLab REST API v4
│   │   ├── gitea.go            Gitea/Forgejo REST API
│   │   └── provider_test.go
│   ├── mirror/                 Migration and push mirror logic
│   │   ├── mirror.go
│   │   └── mirror_test.go
│   └── status/                 Clone status checking
│       ├── status.go
│       └── status_test.go
├── docs/                       Documentation
├── git-config-repos/           Legacy bash script (UNTOUCHED)
├── git-status-pull/            Legacy bash script (UNTOUCHED)
├── git-toolkit.schema.json     v2 JSON Schema
├── git-toolkit.jsonc           Annotated example config
├── go.mod
├── go.sum
└── README.md
```

### Key Design Decisions

- **`pkg/` is the heart** — both CLI and GUI import from here. All business logic lives in `pkg/`.
- **CLI is a thin wrapper** — `cmd/cli/main.go` wires subcommands to `pkg/` functions.
- **GUI calls Go directly** — Wails bindings expose `pkg/` functions to Svelte. No subprocess spawning.
- **Git operations use `os/exec`** — we shell out to the system `git` binary, not libgit2.
- **Provider APIs use `net/http`** — standard Go, no external HTTP client dependencies.

---

## Adding a New Provider

1. Create `pkg/provider/newprovider.go`:

```go
package provider

type NewProvider struct {
    baseURL  string
    username string
    token    string
}

func (p *NewProvider) ListRepos() ([]RepoInfo, error) {
    // Implement API call to list repositories
}

func (p *NewProvider) CreateRepo(name string, private bool) error {
    // Implement API call to create a repository
}

func (p *NewProvider) SetupMirror(repo string, targetURL string) error {
    // Implement mirror API if the provider supports it
}
```

2. Register the provider in `pkg/provider/provider.go`:

```go
func NewFromConfig(source config.Source) (Provider, error) {
    switch source.Provider {
    case "github":
        return &GitHub{...}, nil
    case "newprovider":
        return &NewProvider{...}, nil
    // ...
    }
}
```

3. Add `"newprovider"` to the `provider` enum in `git-toolkit.schema.json`.

4. Write tests in `pkg/provider/newprovider_test.go`.

---

## Adding a New CLI Subcommand

1. Create a new file in `cmd/cli/` or add to `main.go`:

```go
var newCmd = &cobra.Command{
    Use:   "newcommand",
    Short: "Description of the new command",
    RunE: func(cmd *cobra.Command, args []string) error {
        cfg, err := config.Load(configPath)
        if err != nil {
            return err
        }
        // Call pkg/ functions
        return nil
    },
}
```

2. Register in `main.go`: `rootCmd.AddCommand(newCmd)`

---

## Testing

```bash
# Run all tests
go test ./pkg/...

# Run tests with verbose output
go test -v ./pkg/...

# Run tests for a specific package
go test -v ./pkg/config/

# Run a specific test
go test -v -run TestLoadV2Config ./pkg/config/
```

### Test Conventions

- Unit tests live alongside the code they test (`foo_test.go` next to `foo.go`)
- Use table-driven tests for functions with multiple input/output cases
- Mock external calls (git subprocess, HTTP APIs) using interfaces
- Integration tests that need real git repos should create temp directories

---

## Config Schema Evolution

When adding new fields to the configuration:

1. Add the field to the Go struct in `pkg/config/config.go` with `json:"field_name,omitempty"`
2. Add the field to `git-toolkit.schema.json` with a clear description
3. Update `git-toolkit.jsonc` with an example
4. If the field affects v1→v2 migration, update `pkg/config/migrate.go`
5. Update `docs/user-guide.md` config reference table
6. Add tests for the new field in `pkg/config/config_test.go`

**Never bump the version number for additive changes.** Version 2 can grow with optional fields. Only bump to version 3 if breaking changes are needed (renames, removals, type changes).

---

## Release Process

### Building Release Binaries

```bash
# Build all platforms
./scripts/build-all.sh
# Or manually:
GOOS=windows GOARCH=amd64 go build -o dist/git-toolkit-windows-amd64.exe ./cmd/cli
GOOS=darwin  GOARCH=arm64 go build -o dist/git-toolkit-darwin-arm64 ./cmd/cli
GOOS=linux   GOARCH=amd64 go build -o dist/git-toolkit-linux-amd64 ./cmd/cli

# GUI builds (requires Wails)
cd cmd/gui
GOOS=windows wails build
GOOS=darwin  wails build
GOOS=linux   wails build
```

### Creating a Release

1. Update version in code
2. Create a git tag: `git tag v0.1.0`
3. Push tag: `git push origin v0.1.0`
4. Build binaries for all platforms
5. Create GitHub release with the binaries

---

## Code Style

- Follow standard Go conventions (`gofmt`, `go vet`)
- Use `golangci-lint` if available
- Error messages should be lowercase, no trailing punctuation
- Exported functions need doc comments
- Use `context.Context` for operations that may be cancelled (GUI async ops)

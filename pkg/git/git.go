// Package git provides subprocess wrappers for Git operations.
package git

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
)

// CloneOpts configures a git clone operation.
type CloneOpts struct {
	Depth  int    // Shallow clone depth (0 = full clone)
	Branch string // Specific branch to clone
	Bare   bool   // Bare clone (for mirrors)
	Mirror bool   // Mirror clone
}

// RepoStatus holds the parsed output of git status.
type RepoStatus struct {
	Branch    string // Current branch name
	Upstream  string // Upstream tracking branch
	Ahead     int    // Commits ahead of upstream
	Behind    int    // Commits behind upstream
	Modified  int    // Modified files count
	Added     int    // Added (staged) files count
	Deleted   int    // Deleted files count
	Untracked int    // Untracked files count
	Conflicts int    // Conflicted files count
}

// Clone runs git clone with the given options.
func Clone(url, dest string, opts CloneOpts) error {
	args := []string{"clone"}
	if opts.Mirror {
		args = append(args, "--mirror")
	} else if opts.Bare {
		args = append(args, "--bare")
	}
	if opts.Depth > 0 {
		args = append(args, "--depth", strconv.Itoa(opts.Depth))
	}
	if opts.Branch != "" {
		args = append(args, "--branch", opts.Branch)
	}
	args = append(args, url, dest)
	return run(".", args...)
}

// Fetch runs git fetch --all in the given repo.
func Fetch(repoPath string) error {
	return run(repoPath, "fetch", "--all", "--prune")
}

// Pull runs git pull --ff-only in the given repo.
func Pull(repoPath string) error {
	return run(repoPath, "pull", "--ff-only")
}

// Status runs git status and parses the result.
func Status(repoPath string) (RepoStatus, error) {
	out, err := output(repoPath, "status", "--porcelain=v2", "--branch")
	if err != nil {
		return RepoStatus{}, err
	}
	return parseStatus(out), nil
}

// RevCount returns the number of commits ahead/behind the upstream.
// Returns (0, 0, nil) if there's no upstream.
func RevCount(repoPath string) (ahead, behind int, err error) {
	out, err := output(repoPath, "rev-list", "--count", "--left-right", "HEAD...@{upstream}")
	if err != nil {
		// No upstream configured — not an error, just 0/0.
		return 0, 0, nil
	}
	parts := strings.Fields(strings.TrimSpace(out))
	if len(parts) != 2 {
		return 0, 0, nil
	}
	ahead, _ = strconv.Atoi(parts[0])
	behind, _ = strconv.Atoi(parts[1])
	return ahead, behind, nil
}

// RemoteURL returns the URL of the 'origin' remote.
func RemoteURL(repoPath string) (string, error) {
	out, err := output(repoPath, "remote", "get-url", "origin")
	if err != nil {
		return "", err
	}
	return strings.TrimSpace(out), nil
}

// ConfigGet reads a git config value from the given repo.
func ConfigGet(repoPath, key string) (string, error) {
	out, err := output(repoPath, "config", "--get", key)
	if err != nil {
		return "", err
	}
	return strings.TrimSpace(out), nil
}

// ConfigSet sets a git config value in the given repo.
func ConfigSet(repoPath, key, value string) error {
	return run(repoPath, "config", key, value)
}

// IsRepo checks if the given path contains a .git directory or is a bare repo.
func IsRepo(path string) bool {
	gitDir := filepath.Join(path, ".git")
	if info, err := os.Stat(gitDir); err == nil {
		return info.IsDir()
	}
	// Check if it's a bare repo (has HEAD file directly).
	if _, err := os.Stat(filepath.Join(path, "HEAD")); err == nil {
		if _, err := os.Stat(filepath.Join(path, "refs")); err == nil {
			return true
		}
	}
	return false
}

// CurrentBranch returns the current branch name.
func CurrentBranch(repoPath string) (string, error) {
	out, err := output(repoPath, "rev-parse", "--abbrev-ref", "HEAD")
	if err != nil {
		return "", err
	}
	return strings.TrimSpace(out), nil
}

// --- Internal helpers ---

// run executes a git command in the given directory.
func run(dir string, args ...string) error {
	cmd := exec.Command("git", args...)
	cmd.Dir = dir
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return fmt.Errorf("git %s: %w", strings.Join(args, " "), err)
	}
	return nil
}

// output executes a git command and returns its stdout.
func output(dir string, args ...string) (string, error) {
	cmd := exec.Command("git", args...)
	cmd.Dir = dir
	out, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("git %s: %w", strings.Join(args, " "), err)
	}
	return string(out), nil
}

// parseStatus parses git status --porcelain=v2 --branch output.
func parseStatus(out string) RepoStatus {
	var s RepoStatus
	for _, line := range strings.Split(out, "\n") {
		line = strings.TrimSpace(line)
		if line == "" {
			continue
		}

		// Branch headers: # branch.oid, # branch.head, # branch.upstream, # branch.ab
		if strings.HasPrefix(line, "# branch.head ") {
			s.Branch = strings.TrimPrefix(line, "# branch.head ")
		} else if strings.HasPrefix(line, "# branch.upstream ") {
			s.Upstream = strings.TrimPrefix(line, "# branch.upstream ")
		} else if strings.HasPrefix(line, "# branch.ab ") {
			parts := strings.Fields(strings.TrimPrefix(line, "# branch.ab "))
			if len(parts) == 2 {
				s.Ahead, _ = strconv.Atoi(strings.TrimPrefix(parts[0], "+"))
				s.Behind, _ = strconv.Atoi(strings.TrimPrefix(parts[1], "-"))
			}
		} else if strings.HasPrefix(line, "1 ") || strings.HasPrefix(line, "2 ") {
			// Changed entry (ordinary or renamed).
			// Format: 1 XY sub mH mI mW hH hI path
			parts := strings.Fields(line)
			if len(parts) >= 2 {
				xy := parts[1]
				if len(xy) == 2 {
					indexStatus := xy[0]
					workStatus := xy[1]
					if indexStatus != '.' {
						s.Added++ // Staged change.
					}
					if workStatus == 'M' || workStatus == 'A' || workStatus == 'D' {
						s.Modified++ // Working tree change.
					}
					if workStatus == 'D' || indexStatus == 'D' {
						s.Deleted++
					}
				}
			}
		} else if strings.HasPrefix(line, "u ") {
			// Unmerged entry (conflict).
			s.Conflicts++
		} else if strings.HasPrefix(line, "? ") {
			// Untracked file.
			s.Untracked++
		}
	}
	return s
}

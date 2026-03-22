package main

import (
	"fmt"
	"strings"

	"github.com/LuisPalacios/git-toolkit/pkg/config"
	"github.com/LuisPalacios/git-toolkit/pkg/git"
	"github.com/LuisPalacios/git-toolkit/pkg/status"
	"github.com/spf13/cobra"
)

var (
	cloneSource string
	cloneRepo   string
	cloneAll    bool
)

var cloneCmd = &cobra.Command{
	Use:   "clone",
	Short: "Clone repositories",
	Long:  "Clone configured repositories that haven't been cloned yet.",
	RunE: func(cmd *cobra.Command, args []string) error {
		cfgPath := resolveConfigPath()
		if cfgPath == "" {
			cfgPath = config.DefaultV2Path()
		}

		cfg, err := config.Load(cfgPath)
		if err != nil {
			return fmt.Errorf("loading config: %w", err)
		}

		globalFolder := config.ExpandTilde(cfg.Global.Folder)
		cloned := 0
		skipped := 0
		errors := 0

		for sourceName, source := range cfg.Sources {
			if cloneSource != "" && sourceName != cloneSource {
				continue
			}

			acct, ok := cfg.Accounts[source.Account]
			if !ok {
				printError("source %q references unknown account %q", sourceName, source.Account)
				errors++
				continue
			}

			for repoName, repo := range source.Repos {
				if cloneRepo != "" && repoName != cloneRepo {
					continue
				}

				sourceFolder := source.EffectiveFolder(sourceName)
				dest := status.ResolveRepoPath(globalFolder, sourceFolder, repoName, repo)

				if git.IsRepo(dest) {
					if verbose {
						fmt.Printf("  skip %s/%s (already cloned)\n", sourceName, repoName)
					}
					skipped++
					continue
				}

				cloneURL := buildCloneURL(acct, repoName, repo)
				fmt.Printf("  clone %s/%s → %s\n", sourceName, repoName, dest)

				if err := git.Clone(cloneURL, dest, git.CloneOpts{}); err != nil {
					printError("cloning %s/%s: %v", sourceName, repoName, err)
					errors++
					continue
				}

				// Set user.name and user.email.
				userName := repo.Name
				if userName == "" {
					userName = acct.Name
				}
				userEmail := repo.Email
				if userEmail == "" {
					userEmail = acct.Email
				}
				if err := git.ConfigSet(dest, "user.name", userName); err != nil {
					printError("setting user.name: %v", err)
				}
				if err := git.ConfigSet(dest, "user.email", userEmail); err != nil {
					printError("setting user.email: %v", err)
				}

				cloned++
			}
		}

		fmt.Printf("\nCloned: %d, Skipped: %d, Errors: %d\n", cloned, skipped, errors)
		return nil
	},
}

func init() {
	cloneCmd.Flags().StringVar(&cloneSource, "source", "", "clone repos from a specific source only")
	cloneCmd.Flags().StringVar(&cloneRepo, "repo", "", "clone a specific repo only")
	cloneCmd.Flags().BoolVar(&cloneAll, "all", false, "clone all configured repos")
}

// buildCloneURL constructs the clone URL from account + repo info.
// Repo names already contain the org prefix (e.g., "Sumwall/sumwall.browser"),
// so the URL is simply: baseURL/repoName.git or git@host:repoName.git
func buildCloneURL(acct config.Account, repoName string, repo config.Repo) string {
	credType := repo.EffectiveCredentialType(&acct)

	switch credType {
	case "ssh":
		if acct.SSH != nil && acct.SSH.Host != "" {
			return fmt.Sprintf("git@%s:%s.git", acct.SSH.Host, repoName)
		}
		hostname := extractHostname(acct.URL)
		return fmt.Sprintf("git@%s:%s.git", hostname, repoName)

	default: // gcm or token — HTTPS.
		baseURL := strings.TrimSuffix(acct.URL, "/")
		return fmt.Sprintf("%s/%s.git", baseURL, repoName)
	}
}

func extractHostname(rawURL string) string {
	// Simple extraction: strip scheme.
	s := rawURL
	if i := strings.Index(s, "://"); i >= 0 {
		s = s[i+3:]
	}
	if i := strings.IndexByte(s, '/'); i >= 0 {
		s = s[:i]
	}
	if i := strings.IndexByte(s, ':'); i >= 0 {
		s = s[:i]
	}
	return s
}

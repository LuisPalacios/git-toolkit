// git-toolkit CLI — lightweight command-line interface for managing
// Git repositories across multiple accounts and providers.
package main

import (
	"fmt"
	"os"

	"github.com/spf13/cobra"
)

// Build-time variables (set via -ldflags).
var version = "0.1.0-dev"

// Global flags.
var (
	configPath string
	jsonOutput bool
	verbose    bool
)

var rootCmd = &cobra.Command{
	Use:   "git-toolkit",
	Short: "Manage Git repositories across multiple accounts and providers",
	Long: `git-toolkit is a unified tool for managing Git repositories across
multiple accounts and providers (GitHub, GitLab, Gitea, Forgejo, Bitbucket).

It reads configuration from ~/.config/git-toolkit/git-toolkit.json and
provides commands for checking status, cloning, pulling, and migrating repos.`,
	SilenceUsage: true,
}

func init() {
	rootCmd.PersistentFlags().StringVar(&configPath, "config", "", "path to config file (default: ~/.config/git-toolkit/git-toolkit.json)")
	rootCmd.PersistentFlags().BoolVar(&jsonOutput, "json", false, "output in JSON format")
	rootCmd.PersistentFlags().BoolVar(&verbose, "verbose", false, "verbose output")

	rootCmd.AddCommand(versionCmd)
	rootCmd.AddCommand(configCmd)
	rootCmd.AddCommand(statusCmd)
	rootCmd.AddCommand(cloneCmd)
	rootCmd.AddCommand(pullCmd)
	rootCmd.AddCommand(migrateCmd)
}

func main() {
	if err := rootCmd.Execute(); err != nil {
		os.Exit(1)
	}
}

// resolveConfigPath returns the config path to use.
func resolveConfigPath() string {
	if configPath != "" {
		return configPath
	}
	return ""
}

// printError prints an error message to stderr.
func printError(format string, args ...any) {
	fmt.Fprintf(os.Stderr, "error: "+format+"\n", args...)
}

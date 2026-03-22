package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	"github.com/LuisPalacios/git-toolkit/pkg/config"
	"github.com/spf13/cobra"
)

var initForce bool

var initCmd = &cobra.Command{
	Use:   "init",
	Short: "Create a new git-toolkit configuration",
	Long: `Creates a new git-toolkit.json configuration file with global settings.

If the file already exists, use --force to overwrite it.
After init, use 'git-toolkit account add' and 'git-toolkit source add' to configure
your accounts and repositories.`,
	RunE: func(cmd *cobra.Command, args []string) error {
		path := configFilePath()

		// Check if config already exists.
		if _, err := os.Stat(path); err == nil && !initForce {
			return fmt.Errorf("config already exists at %s (use --force to overwrite)", path)
		}

		reader := bufio.NewReader(os.Stdin)

		// Ask for root folder.
		fmt.Print("Root folder for all git clones [~/git]: ")
		folder, _ := reader.ReadString('\n')
		folder = strings.TrimSpace(folder)
		if folder == "" {
			folder = "~/git"
		}

		// Ask for credential store.
		credStore := detectCredentialStore()
		fmt.Printf("Credential store detected: %s\n", credStore)

		// Ask for SSH.
		fmt.Print("Enable SSH credential management? [y/N]: ")
		sshAnswer, _ := reader.ReadString('\n')
		sshEnabled := strings.HasPrefix(strings.ToLower(strings.TrimSpace(sshAnswer)), "y")

		// Build config.
		cfg := &config.Config{
			Schema:  "https://raw.githubusercontent.com/LuisPalacios/git-toolkit/main/git-toolkit.schema.json",
			Version: 2,
			Global: config.GlobalConfig{
				Folder: folder,
				CredentialSSH: &config.SSHGlobal{
					Enabled:   sshEnabled,
					SSHFolder: "~/.ssh",
				},
				CredentialGCM: &config.GCMGlobal{
					Enabled:         true,
					Helper:          "manager",
					CredentialStore: credStore,
				},
			},
			Accounts: make(map[string]config.Account),
			Sources:  make(map[string]config.Source),
		}

		if err := config.Save(cfg, path); err != nil {
			return fmt.Errorf("saving config: %w", err)
		}

		fmt.Printf("\nConfiguration created at %s\n", path)
		fmt.Println("\nNext steps:")
		fmt.Println("  1. Add an account:")
		fmt.Println("     git-toolkit account add my-github \\")
		fmt.Println("       --provider github \\")
		fmt.Println("       --url https://github.com \\")
		fmt.Println("       --username YourUser \\")
		fmt.Println("       --name \"Your Name\" \\")
		fmt.Println("       --email your@email.com")
		fmt.Println("")
		fmt.Println("  2. Add a source:")
		fmt.Println("     git-toolkit source add my-github --account my-github")
		fmt.Println("")
		fmt.Println("  3. Add repos:")
		fmt.Println("     git-toolkit repo add my-github \"YourUser/repo-name\"")
		fmt.Println("")
		fmt.Println("  Or migrate from an existing git-config-repos.json:")
		fmt.Println("     git-toolkit migrate")

		return nil
	},
}

func init() {
	rootCmd.AddCommand(initCmd)
	initCmd.Flags().BoolVar(&initForce, "force", false, "overwrite existing config")
}

// detectCredentialStore returns the OS-appropriate credential store.
func detectCredentialStore() string {
	switch {
	case isWindows():
		return "wincredman"
	case isDarwin():
		return "keychain"
	default:
		return "secretservice"
	}
}

func isWindows() bool {
	return os.PathSeparator == '\\' || strings.Contains(strings.ToLower(os.Getenv("OS")), "windows")
}

func isDarwin() bool {
	// Check for macOS-specific paths.
	_, err := os.Stat("/System/Library")
	return err == nil
}

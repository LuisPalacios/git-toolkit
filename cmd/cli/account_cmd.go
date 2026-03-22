package main

import (
	"encoding/json"
	"fmt"
	"os"
	"text/tabwriter"

	"github.com/LuisPalacios/git-toolkit/pkg/config"
	"github.com/spf13/cobra"
)

var accountCmd = &cobra.Command{
	Use:   "account",
	Short: "Manage accounts",
}

// --- account list ---

var accountListCmd = &cobra.Command{
	Use:   "list",
	Short: "List all accounts",
	RunE: func(cmd *cobra.Command, args []string) error {
		cfg, err := loadConfig()
		if err != nil {
			return err
		}

		if jsonOutput {
			data, _ := json.MarshalIndent(cfg.Accounts, "", "    ")
			fmt.Fprintln(os.Stdout, string(data))
			return nil
		}

		w := tabwriter.NewWriter(os.Stdout, 0, 0, 2, ' ', 0)
		fmt.Fprintf(w, "ACCOUNT\tPROVIDER\tURL\tUSERNAME\tDEFAULT CRED\n")
		fmt.Fprintf(w, "───────\t────────\t───\t────────\t────────────\n")
		for key, acct := range cfg.Accounts {
			fmt.Fprintf(w, "%s\t%s\t%s\t%s\t%s\n", key, acct.Provider, acct.URL, acct.Username, acct.DefaultCredentialType)
		}
		return w.Flush()
	},
}

// --- account show ---

var accountShowCmd = &cobra.Command{
	Use:   "show <key>",
	Short: "Show account details",
	Args:  cobra.ExactArgs(1),
	RunE: func(cmd *cobra.Command, args []string) error {
		cfg, err := loadConfig()
		if err != nil {
			return err
		}

		acct, ok := cfg.GetAccountByKey(args[0])
		if !ok {
			return fmt.Errorf("account %q not found", args[0])
		}

		data, _ := json.MarshalIndent(acct, "", "    ")
		fmt.Fprintln(os.Stdout, string(data))
		return nil
	},
}

// --- account add ---

var (
	acctProvider   string
	acctURL        string
	acctUsername   string
	acctName       string
	acctEmail      string
	acctDefBranch  string
	acctDefCred    string
)

var accountAddCmd = &cobra.Command{
	Use:   "add <key>",
	Short: "Add a new account",
	Args:  cobra.ExactArgs(1),
	RunE: func(cmd *cobra.Command, args []string) error {
		cfg, err := loadOrCreateConfig()
		if err != nil {
			return err
		}

		acct := config.Account{
			Provider:              acctProvider,
			URL:                   acctURL,
			Username:              acctUsername,
			Name:                  acctName,
			Email:                 acctEmail,
			DefaultBranch:         acctDefBranch,
			DefaultCredentialType: acctDefCred,
		}

		if err := cfg.AddAccount(args[0], acct); err != nil {
			return err
		}

		return saveConfig(cfg)
	},
}

// --- account update ---

var accountUpdateCmd = &cobra.Command{
	Use:   "update <key>",
	Short: "Update an existing account",
	Args:  cobra.ExactArgs(1),
	RunE: func(cmd *cobra.Command, args []string) error {
		cfg, err := loadConfig()
		if err != nil {
			return err
		}

		acct, ok := cfg.GetAccountByKey(args[0])
		if !ok {
			return fmt.Errorf("account %q not found", args[0])
		}

		// Apply only flags that were explicitly set.
		if cmd.Flags().Changed("provider") {
			acct.Provider = acctProvider
		}
		if cmd.Flags().Changed("url") {
			acct.URL = acctURL
		}
		if cmd.Flags().Changed("username") {
			acct.Username = acctUsername
		}
		if cmd.Flags().Changed("name") {
			acct.Name = acctName
		}
		if cmd.Flags().Changed("email") {
			acct.Email = acctEmail
		}
		if cmd.Flags().Changed("default-branch") {
			acct.DefaultBranch = acctDefBranch
		}
		if cmd.Flags().Changed("default-credential-type") {
			acct.DefaultCredentialType = acctDefCred
		}

		if err := cfg.UpdateAccount(args[0], acct); err != nil {
			return err
		}

		return saveConfig(cfg)
	},
}

// --- account delete ---

var accountDeleteCmd = &cobra.Command{
	Use:   "delete <key>",
	Short: "Delete an account",
	Args:  cobra.ExactArgs(1),
	RunE: func(cmd *cobra.Command, args []string) error {
		cfg, err := loadConfig()
		if err != nil {
			return err
		}

		if err := cfg.DeleteAccount(args[0]); err != nil {
			return err
		}

		return saveConfig(cfg)
	},
}

func init() {
	accountCmd.AddCommand(accountListCmd)
	accountCmd.AddCommand(accountShowCmd)
	accountCmd.AddCommand(accountAddCmd)
	accountCmd.AddCommand(accountUpdateCmd)
	accountCmd.AddCommand(accountDeleteCmd)

	// Flags for add.
	accountAddCmd.Flags().StringVar(&acctProvider, "provider", "", "provider type (github, gitlab, gitea, forgejo, bitbucket, generic)")
	accountAddCmd.Flags().StringVar(&acctURL, "url", "", "server URL (e.g., https://github.com)")
	accountAddCmd.Flags().StringVar(&acctUsername, "username", "", "account username")
	accountAddCmd.Flags().StringVar(&acctName, "name", "", "git user.name")
	accountAddCmd.Flags().StringVar(&acctEmail, "email", "", "git user.email")
	accountAddCmd.Flags().StringVar(&acctDefBranch, "default-branch", "main", "default branch")
	accountAddCmd.Flags().StringVar(&acctDefCred, "default-credential-type", "gcm", "default credential type (gcm, ssh, token)")
	accountAddCmd.MarkFlagRequired("provider")
	accountAddCmd.MarkFlagRequired("url")
	accountAddCmd.MarkFlagRequired("username")
	accountAddCmd.MarkFlagRequired("name")
	accountAddCmd.MarkFlagRequired("email")

	// Flags for update (same names, no required).
	accountUpdateCmd.Flags().StringVar(&acctProvider, "provider", "", "provider type")
	accountUpdateCmd.Flags().StringVar(&acctURL, "url", "", "server URL")
	accountUpdateCmd.Flags().StringVar(&acctUsername, "username", "", "username")
	accountUpdateCmd.Flags().StringVar(&acctName, "name", "", "git user.name")
	accountUpdateCmd.Flags().StringVar(&acctEmail, "email", "", "git user.email")
	accountUpdateCmd.Flags().StringVar(&acctDefBranch, "default-branch", "", "default branch")
	accountUpdateCmd.Flags().StringVar(&acctDefCred, "default-credential-type", "", "default credential type")
}

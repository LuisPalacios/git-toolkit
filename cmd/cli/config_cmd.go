package main

import (
	"encoding/json"
	"fmt"
	"os"

	"github.com/LuisPalacios/git-toolkit/pkg/config"
	"github.com/spf13/cobra"
)

var configCmd = &cobra.Command{
	Use:   "config",
	Short: "Manage configuration",
}

var configShowCmd = &cobra.Command{
	Use:   "show",
	Short: "Display the current configuration",
	RunE: func(cmd *cobra.Command, args []string) error {
		cfgPath := resolveConfigPath()
		if cfgPath == "" {
			cfgPath = config.DefaultV2Path()
		}

		cfg, err := config.Load(cfgPath)
		if err != nil {
			return fmt.Errorf("loading config from %s: %w", cfgPath, err)
		}

		data, err := json.MarshalIndent(cfg, "", "    ")
		if err != nil {
			return fmt.Errorf("marshalling config: %w", err)
		}

		fmt.Fprintln(os.Stdout, string(data))
		return nil
	},
}

var configPathCmd = &cobra.Command{
	Use:   "path",
	Short: "Print the configuration file path",
	Run: func(cmd *cobra.Command, args []string) {
		cfgPath := resolveConfigPath()
		if cfgPath == "" {
			cfgPath = config.DefaultV2Path()
		}
		fmt.Println(cfgPath)
	},
}

func init() {
	configCmd.AddCommand(configShowCmd)
	configCmd.AddCommand(configPathCmd)
}

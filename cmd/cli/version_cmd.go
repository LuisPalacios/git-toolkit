package main

import (
	"fmt"

	"github.com/spf13/cobra"
)

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Print the version of git-toolkit",
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Printf("git-toolkit %s\n", version)
	},
}

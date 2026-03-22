package main

import (
	"fmt"

	"github.com/LuisPalacios/git-toolkit/pkg/config"
	"github.com/LuisPalacios/git-toolkit/pkg/git"
	"github.com/LuisPalacios/git-toolkit/pkg/status"
	"github.com/spf13/cobra"
)

var (
	pullSource string
	pullRepo   string
	pullAll    bool
)

var pullCmd = &cobra.Command{
	Use:   "pull",
	Short: "Pull repositories that are behind upstream",
	Long:  "Pull (fast-forward only) repositories that are behind their upstream.",
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
		pulled := 0
		skipped := 0
		errors := 0

		for sourceName, source := range cfg.Sources {
			if pullSource != "" && sourceName != pullSource {
				continue
			}

			sourceFolder := source.EffectiveFolder(sourceName)
			for repoName, repo := range source.Repos {
				if pullRepo != "" && repoName != pullRepo {
					continue
				}

				dest := status.ResolveRepoPath(globalFolder, sourceFolder, repoName, repo)

				if !git.IsRepo(dest) {
					if verbose {
						fmt.Printf("  skip %s/%s (not cloned)\n", sourceName, repoName)
					}
					skipped++
					continue
				}

				// Check status first — only pull if behind.
				rs := status.Check(dest)
				if rs.State == status.Clean || rs.State == status.Ahead || rs.State == status.NoUpstream {
					if verbose {
						fmt.Printf("  skip %s/%s (%s)\n", sourceName, repoName, rs.State)
					}
					skipped++
					continue
				}

				if rs.State == status.Dirty || rs.State == status.Conflict {
					fmt.Printf("  skip %s/%s (%s — resolve manually)\n", sourceName, repoName, rs.State)
					skipped++
					continue
				}

				// Behind or Diverged — attempt pull.
				fmt.Printf("  pull %s/%s (%d behind)\n", sourceName, repoName, rs.Behind)
				if err := git.Pull(dest); err != nil {
					printError("pulling %s/%s: %v", sourceName, repoName, err)
					errors++
					continue
				}
				pulled++
			}
		}

		fmt.Printf("\nPulled: %d, Skipped: %d, Errors: %d\n", pulled, skipped, errors)
		return nil
	},
}

func init() {
	pullCmd.Flags().StringVar(&pullSource, "source", "", "pull repos from a specific source only")
	pullCmd.Flags().StringVar(&pullRepo, "repo", "", "pull a specific repo only")
	pullCmd.Flags().BoolVar(&pullAll, "all", false, "pull all configured repos")
}

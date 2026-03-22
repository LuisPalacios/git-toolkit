package main

import (
	"encoding/json"
	"fmt"
	"os"
	"sort"
	"text/tabwriter"

	"github.com/LuisPalacios/git-toolkit/pkg/config"
	"github.com/LuisPalacios/git-toolkit/pkg/status"
	"github.com/spf13/cobra"
)

var (
	statusSource string
	statusRepo   string
)

var statusCmd = &cobra.Command{
	Use:   "status",
	Short: "Show sync status of all repositories",
	RunE: func(cmd *cobra.Command, args []string) error {
		cfgPath := resolveConfigPath()
		if cfgPath == "" {
			cfgPath = config.DefaultV2Path()
		}

		cfg, err := config.Load(cfgPath)
		if err != nil {
			return fmt.Errorf("loading config: %w", err)
		}

		results := status.CheckAll(cfg)

		// Filter by source/repo if specified.
		if statusSource != "" || statusRepo != "" {
			var filtered []status.RepoStatus
			for _, r := range results {
				if statusSource != "" && r.Source != statusSource {
					continue
				}
				if statusRepo != "" && r.Repo != statusRepo {
					continue
				}
				filtered = append(filtered, r)
			}
			results = filtered
		}

		// Sort by source, then repo.
		sort.Slice(results, func(i, j int) bool {
			if results[i].Source != results[j].Source {
				return results[i].Source < results[j].Source
			}
			return results[i].Repo < results[j].Repo
		})

		if jsonOutput {
			return printStatusJSON(results)
		}
		return printStatusTable(results)
	},
}

func init() {
	statusCmd.Flags().StringVar(&statusSource, "source", "", "filter by source name")
	statusCmd.Flags().StringVar(&statusRepo, "repo", "", "filter by repo name")
}

func printStatusJSON(results []status.RepoStatus) error {
	data, err := json.MarshalIndent(results, "", "    ")
	if err != nil {
		return err
	}
	fmt.Fprintln(os.Stdout, string(data))
	return nil
}

func printStatusTable(results []status.RepoStatus) error {
	if len(results) == 0 {
		fmt.Println("No repositories found.")
		return nil
	}

	w := tabwriter.NewWriter(os.Stdout, 0, 0, 2, ' ', 0)
	fmt.Fprintf(w, "STATUS\tSOURCE\tREPO\tDETAILS\n")
	fmt.Fprintf(w, "──────\t──────\t────\t───────\n")

	for _, r := range results {
		details := formatDetails(r)
		fmt.Fprintf(w, "%s %s\t%s\t%s\t%s\n",
			r.State.Symbol(), r.State.String(),
			r.Source, r.Repo, details)
	}

	return w.Flush()
}

func formatDetails(r status.RepoStatus) string {
	switch r.State {
	case status.Behind:
		return fmt.Sprintf("%d behind", r.Behind)
	case status.Ahead:
		return fmt.Sprintf("%d ahead", r.Ahead)
	case status.Diverged:
		return fmt.Sprintf("%d ahead, %d behind", r.Ahead, r.Behind)
	case status.Dirty:
		parts := []string{}
		if r.Modified > 0 {
			parts = append(parts, fmt.Sprintf("%d modified", r.Modified))
		}
		if r.Untracked > 0 {
			parts = append(parts, fmt.Sprintf("%d untracked", r.Untracked))
		}
		if len(parts) == 0 {
			return "has changes"
		}
		return joinParts(parts)
	case status.Conflict:
		return fmt.Sprintf("%d conflicts", r.Conflicts)
	case status.Error:
		return r.ErrorMsg
	default:
		return ""
	}
}

func joinParts(parts []string) string {
	result := ""
	for i, p := range parts {
		if i > 0 {
			result += ", "
		}
		result += p
	}
	return result
}

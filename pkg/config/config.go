// Package config handles loading, saving, and migrating git-toolkit configuration files.
package config

// Config represents the top-level git-toolkit configuration (v2 format).
type Config struct {
	Schema   string              `json:"$schema,omitempty"`
	Version  int                 `json:"version"`
	Global   GlobalConfig        `json:"global"`
	Accounts map[string]Account  `json:"accounts"`
	Sources  map[string]Source   `json:"sources"`
}

// GlobalConfig holds global settings.
type GlobalConfig struct {
	Folder        string     `json:"folder"`
	CredentialSSH *SSHGlobal `json:"credential_ssh,omitempty"`
	CredentialGCM *GCMGlobal `json:"credential_gcm,omitempty"`
}

// SSHGlobal holds global SSH credential management settings.
type SSHGlobal struct {
	Enabled   bool   `json:"enabled"`
	SSHFolder string `json:"ssh_folder,omitempty"`
}

// GCMGlobal holds global Git Credential Manager settings.
type GCMGlobal struct {
	Enabled         bool   `json:"enabled"`
	Helper          string `json:"helper,omitempty"`
	CredentialStore string `json:"credential_store,omitempty"`
}

// Account represents a Git provider account — WHO you are on a server.
// The map key in Config.Accounts is the human-friendly account ID,
// also used as the default first-level clone folder name.
type Account struct {
	Provider              string     `json:"provider"`
	URL                   string     `json:"url"`
	Username              string     `json:"username"`
	Name                  string     `json:"name"`
	Email                 string     `json:"email"`
	DefaultBranch         string     `json:"default_branch,omitempty"`
	DefaultCredentialType string     `json:"default_credential_type,omitempty"`
	SSH                   *SSHConfig `json:"ssh,omitempty"`
	GCM                   *GCMConfig `json:"gcm,omitempty"`
}

// SSHConfig holds SSH authentication settings for an account.
type SSHConfig struct {
	Host     string `json:"host,omitempty"`
	Hostname string `json:"hostname,omitempty"`
	KeyType  string `json:"key_type,omitempty"`
}

// GCMConfig holds Git Credential Manager settings for an account.
type GCMConfig struct {
	Provider    string `json:"provider,omitempty"`
	UseHTTPPath bool   `json:"use_http_path,omitempty"`
}

// Source represents WHAT you clone — references an account, contains repos.
// The map key is the source ID. The clone folder defaults to the source key
// unless overridden by the Folder field.
type Source struct {
	Account string          `json:"account"`
	Folder  string          `json:"folder,omitempty"`
	Repos   map[string]Repo `json:"repos"`
}

// EffectiveFolder returns the clone folder for this source.
// If Folder is set, use it. Otherwise, use the source key (passed as argument).
func (s *Source) EffectiveFolder(sourceKey string) string {
	if s.Folder != "" {
		return s.Folder
	}
	return sourceKey
}

// Repo represents a single repository configuration.
// Repo names use "org/repo" format — the org part becomes the second-level folder.
type Repo struct {
	CredentialType string `json:"credential_type,omitempty"`
	Name           string `json:"name,omitempty"`
	Email          string `json:"email,omitempty"`
	IdFolder       string `json:"id_folder,omitempty"`    // overrides 2nd level dir (org). Default: part before / in repo key.
	CloneFolder    string `json:"clone_folder,omitempty"` // overrides 3rd level dir (clone name). If absolute (/ ~ ../), replaces entire path.
}

// EffectiveCredentialType returns the credential type for this repo.
// If set on the repo, use it. Otherwise, inherit from the account default.
func (r *Repo) EffectiveCredentialType(acct *Account) string {
	if r.CredentialType != "" {
		return r.CredentialType
	}
	if acct != nil {
		return acct.DefaultCredentialType
	}
	return ""
}

// GetAccount resolves the account for a source. Returns nil if not found.
func (c *Config) GetAccount(sourceName string) *Account {
	src, ok := c.Sources[sourceName]
	if !ok {
		return nil
	}
	acct, ok := c.Accounts[src.Account]
	if !ok {
		return nil
	}
	return &acct
}

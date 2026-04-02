#ifndef HELPERS_H
#define HELPERS_H

#include <json.hpp>
#include "Types.h"

#include <string>
#include <vector>
#include <chrono>
#include <unordered_set>

using json = nlohmann::json;

std::string get_dir_name(const std::string& path);
std::string extract_promotion_strategy(const std::string& dirname);

namespace TypeConversions {
	MatchingType s_to_matching_type(const std::string& type);

	PromotionStrategy s_to_promotion_strategy(const std::string& type);

	ExclusionRule s_to_exclusion_rule(const std::string& excl_type, const std::string& excl_pattern);
	ExclusionType s_to_exclusion_type(const std::string& type);
	json exclusion_rules_to_json(const std::vector<ExclusionRule>& rules);
	std::string exclusion_type_to_s(const ExclusionType& type);
};

namespace ArgParsing {
	static const std::unordered_map<std::string, std::string> flag_aliases = {
		{"v", "version"},
		{"h", "help"},
		{"r", "root"},
		{"f", "force"},
		{"e", "exact"},
		{"p", "prefix"},
		{"s", "suffix"},
		{"c", "contains"},
		{"ra", "recently_accessed"},
		{"fb", "frequency_based"}
	};
	static const std::vector<std::string> full_flag_names = {
		"version",
		"help",
		"root",
		"force",
		"exact",
		"prefix",
		"suffix",
		"contains",
		"recently_accessed",
		"frequency_based",
		"[bypass]" // converted version of '--'
	};
	static const std::unordered_map<std::string, std::vector<std::pair<std::string, bool>>> valid_flags = {
		// Build/rebuild/refresh command flags
		// (flag, requires value)
	{"", {{"version", false}, {"[bypass]", true}}},
	{"build", {{"root", true}, {"force", false}}},
	{"rebuild", {{"root", true}, {"force", false}}},
	{"refresh", {{"root", true}}}
	};
	static const std::unordered_map<std::string, std::string> raw_flag_to_implied = {
		{"--", "--[bypass]"}
	};
	// First token after "dv": if it matches here, argv is passed through without Dirvana flag parsing
	// (e.g. cp -r, rm -rf). Omit Dirvana subcommands: build, rebuild, refresh, install, add, delete, list, show.
	static const std::unordered_set<std::string> system_shell_commands = {
		"awk", "bash", "brew", "bun", "bunx", "cat", "cd", "chflags", "chmod", "chown", "cp", "curl", "cut",
		"date", "dd", "diff", "dig", "dirname", "diskutil", "docker", "du", "ed", "env", "ex", "false", "fd",
		"ffmpeg", "fgrep", "file", "find", "fish", "g++", "gcc", "gem", "gh", "git", "gmake", "grep", "gunzip",
		"gzip", "head", "hg", "htop", "id", "jq", "kill", "killall", "less", "ln", "ls", "lsof", "make", "man",
		"md5", "md5sum", "mkdir", "mv", "nano", "nice", "node", "npm", "npx", "open", "openssl", "perl", "pgrep",
		"ping", "pnpm", "pod", "python", "python3", "readlink", "realpath", "rg", "rm", "rmdir", "rsync", "ruby",
		"say", "scp", "sed", "seq", "sh", "sort", "ssh", "stat", "sudo", "svn", "swift", "tail", "tar", "tee",
		"terraform", "test", "time", "top", "touch", "tr", "true", "uname", "unzip", "vim", "vi", "wc", "wget",
		"which", "whoami", "xargs", "xattr", "yarn", "yes", "zip", "zsh"
	};
	std::pair<bool, Flag> build_flag(const std::vector<std::string>& flag_parts, const std::string& cmd);
	std::tuple<bool, std::vector<std::string>, std::vector<Flag>> process_args(int argc, char* argv[]);
	bool validate_flag(const Flag& flag);
	std::string get_flag_value(const std::vector<Flag>& flags, const std::string& flag_name, const std::string& default_value = "");
	bool has_flag(const std::vector<Flag>& flags, const std::string& flag_name);
}

namespace Time {
	inline long long now() {
		auto now = std::chrono::system_clock::now();
		auto duration_since_epoch = now.time_since_epoch();
		auto micros_since_epoch = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch);

		return micros_since_epoch.count();
	};
}

#endif // HELPERS_H
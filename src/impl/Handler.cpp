#include "Handler.h"

#include <fstream>
#include <filesystem>
#include <mach-o/dyld.h>
#include <unistd.h>

Handler::Handler(Database& db, const std::string& version) : db(db), version(version) {}

int Handler::handle_tab(int argc, char* argv[]) {
	// Need at least 4 arguments: dv_binary, --tab, dv, partial_path
	if (argc < 4) {
		std::cerr << "Tab completion requires at least a partial path" << std::endl;
		return 1;
	}

	
	
	// Last argument is the partial path
	std::string partial = argv[argc - 1];

	// Check if we need to do lazy, in-memory file completion
	// Our heurisitc is if the last char in the partial path is a '/'
	if (partial.back() == '/') {
		// Lazy, in-memory file completion
		std::vector<std::string> matches = db.get_paths_table().collect_files(partial);
		for (const auto& match : matches)
			std::cout << match << std::endl;
		return 0;
	}
	
	// Get matches for the partial path
	std::vector<std::string> matches = db.get_paths_table().query(partial);
	
	// Check if there are commands/inputs between "dv" and the partial path
	std::string prefix = "";
	for (int i = 3; i < argc - 1; i++) {
		if (i > 3) prefix += " ";
		prefix += argv[i];
	}

	// Print the matches with appropriate prefixes for zsh completion
	for (const auto& match : matches)
		std::cout << match << std::endl;
	
	return 0;
}


int Handler::handle_enter(std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// If --enter was called with no arguments, that is the eqivalent of "cd"
	// where we want to cd to home dir
	if (commands.empty() and flags.empty()) {
		std::cout << "cd ~" << std::endl;
		return 0;
	}

	// Check if a subcommand-less flag was passed (e.g. "--version" ("-v"))
	if (ArgParsing::has_flag(flags, "version")) {
		std::cout << "echo Dirvana version " << version << std::endl;
		return 0;
	}

	bool bypass = ArgParsing::has_flag(flags, "[bypass]");
	std::string first_token = bypass ? ArgParsing::get_flag_value(flags, "[bypass]") : (not commands.empty() ? commands[0] : "");

	// Check if a subcommand was passed
	if (not bypass) {
		if (first_token == "build" or first_token == "rebuild")
			return Subcommands::handle_re_build(*this, commands, flags);
		else if (first_token == "refresh")
			return Subcommands::handle_refresh(*this, commands, flags);
		else if (first_token == "install")
			return Subcommands::handle_install(*this, commands, flags);
		else if (first_token == "init")
			return Subcommands::handle_init(*this, commands, flags);
		else if (first_token == "add")
			return Subcommands::handle_add(*this, commands, flags);
		else if (first_token == "delete")
			return Subcommands::handle_delete(*this, commands, flags);
		else if (first_token == "list")
			return Subcommands::handle_list(*this, commands, flags);
		else if (first_token == "show")
			return Subcommands::handle_show(*this, commands, flags);
	}

	// If we are here, need to handle a shortcut or a path. We prioritize shortcuts over paths

	std::vector<std::string> matches = db.get_shortcuts_table().query(first_token);
	std::string command = matches.empty() ? "" : matches[0];
	if (not command.empty()) {
		// Build the arguments for the shortcut
		std::string args = "";
		for (size_t i = 1; i < commands.size() - 1; i++)
			args += " " + commands[i];

		// Try to complete the last command as a path
		std::string last_token = commands.size() > 1 ? commands.back() : "";
		if (not last_token.empty() and 
				last_token.find('/') == std::string::npos and 
				last_token.find('~') == std::string::npos) {
			
			// Partial path, need to complete
			std::vector<std::string> matches = db.get_paths_table().query(last_token);
			if (!matches.empty())
				last_token = matches[0];
		}

		// Check if the completion needs to be embedded in the command
		// We use {} to embed the completion in the command
		const size_t start = command.find("{}");
		if (start != std::string::npos)
			// Replace the {} with the last token
			command = command.replace(start, 2, last_token);
		else
			// Add the last token to the command
			command += " " + last_token;

		// Execute the shortcut
		std::cout << command << std::endl;

		// Update the database with the accessed path
		db.get_shortcuts_table().access(command);

		return 0;
	}

	// If we are here, we need to handle a path
	
	// Last token (or arg passed to --) is the path to complete
	std::string path = !commands.empty() ? commands.back() : ArgParsing::get_flag_value(flags, "[bypass]");
	// Check if path is full path or partial
	if (path.find('/') == std::string::npos and 
		path.find('~') == std::string::npos and 
		path.find('.') == std::string::npos) {
		// Partial path, need to complete
		std::vector<std::string> matches = db.get_paths_table().query(path);
		if (matches.empty()) {
			// 'cd' to the path if no matches found for entries like "~", "..", etc.
			std::cout << "cd " << path << std::endl;
			return 0;
		}
		// Use the first match
		path = matches[0];
	}

	// Update the database with the accessed path
	db.get_paths_table().access(path);
	
	// Now we need to assemble the final command to output.
	// Arguments look something like: dv-binary --enter dv [...] [path]
	std::string prefix = "";
	if (commands.size() > 1) {
		// Build the prefix from all arguments except the last one (and any "--" delimiters)
		for (size_t i = 0; i < commands.size() - 1; i++) {
			if (i > 0) prefix += " ";
			prefix += commands[i] != "--" ? commands[i] : "";
		}
	}
		
	// Decide what to output based on presence of prefix args and matches
	// If no prefix args, just 'cd' to the matched path
	if (prefix.empty())
		// No output, just 'cd' to the path
		std::cout << "cd " << path << std::endl;
	else
		// Execute the output with the path
		std::cout << prefix << " " << path << std::endl;

	return 0;
}

int Handler::Subcommands::handle_re_build(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for build/rebuild:
	std::string init_path = ArgParsing::get_flag_value(flags, "root", handler.get_init_path());
	bool force = ArgParsing::has_flag(flags, "force");
				
	if (handler.db.build(init_path, force)) {
		std::cout << "echo Build from " << init_path << " complete" << std::endl;
		return 0;
	} else
		return 1;
}

int Handler::Subcommands::handle_refresh(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for refresh:
	std::string init_path = ArgParsing::get_flag_value(flags, "root", handler.get_init_path());

	if (handler.db.refresh(init_path)) {
		std::cout << "echo Refresh from " << init_path << " complete" << std::endl;
		return 0;
	} else
		return 1;
}

int Handler::Subcommands::handle_install(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for install:
	std::string version = ArgParsing::get_flag_value(flags, "version", "latest");
	if (version == "latest")
		version = handler.version;
	// Curl command to download and run the installation script
	std::string curl_command = std::format(
		"curl -fsSL https://raw.githubusercontent.com/jameskendrick/dirvana/{}/docs/install.sh | bash",
		version.c_str()
	);
	std::cout << "echo Updating Dirvana to version " << version << "..." << std::endl;
	std::cout << "echo Running: " << curl_command << std::endl;
	return 0;
}

static std::string get_executable_path() {
	char buffer[4096];
	uint32_t size = sizeof(buffer);
	if (_NSGetExecutablePath(buffer, &size) != 0)
		return "";
	std::error_code ec;
	auto canonical = std::filesystem::canonical(buffer, ec);
	if (ec) return buffer;
	return canonical.string();
}

static std::string find_first_on_path(const std::string& name) {
	const char* path_env = std::getenv("PATH");
	if (!path_env) return "";
	std::string path_str = path_env;
	size_t start = 0;
	while (start <= path_str.size()) {
		size_t end = path_str.find(':', start);
		std::string dir = path_str.substr(start, end == std::string::npos ? std::string::npos : end - start);
		start = (end == std::string::npos) ? path_str.size() + 1 : end + 1;
		if (dir.empty()) continue;
		std::string candidate = dir + "/" + name;
		if (access(candidate.c_str(), X_OK) == 0) {
			std::error_code ec;
			auto canonical = std::filesystem::canonical(candidate, ec);
			return ec ? candidate : canonical.string();
		}
	}
	return "";
}

int Handler::Subcommands::handle_init(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	const char* home_env = std::getenv("HOME");
	if (!home_env) {
		std::cerr << "HOME environment variable not set" << std::endl;
		return 1;
	}
	std::string home = home_env;
	std::string zshrc_path = home + "/.zshrc";
	std::string init_path = ArgParsing::get_flag_value(flags, "root", home);

	// Detect installation method from where the binary actually lives
	std::string exe_path = get_executable_path();
	bool is_local_bin = exe_path.find(home + "/.local/bin/") != std::string::npos;

	// Warn if another dv-binary earlier on PATH will shadow this one — common when
	// switching between the curl|bash installer (~/.local/bin) and Homebrew.
	std::string first_on_path = find_first_on_path("dv-binary");
	if (!first_on_path.empty() && !exe_path.empty() && first_on_path != exe_path) {
		std::cerr << "Warning: another dv-binary is earlier on your PATH and will be used instead of this one." << std::endl;
		std::cerr << "  Running:    " << exe_path << std::endl;
		std::cerr << "  Shadowed by: " << first_on_path << std::endl;
		std::cerr << "Remove the shadowing binary (e.g. `rm " << first_on_path << " && hash -r`) so `dv-binary` resolves to this install." << std::endl;
	}

	// Read existing .zshrc to avoid duplicating any block
	std::string zshrc_content;
	{
		std::ifstream in(zshrc_path);
		if (in.is_open())
			zshrc_content = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	}

	// Always install _dv and add the completion block — Homebrew adds its own _dv too but
	// compinit picks the first match in fpath, so duplication is harmless. Skipping this on
	// Homebrew was a mistake: Homebrew adds to fpath but doesn't run compinit.
	std::string completions_dir = home + "/.zsh/completions";
	std::error_code ec;
	std::filesystem::create_directories(completions_dir, ec);

	std::string completion_file = completions_dir + "/_dv";
	if (!std::filesystem::exists(completion_file)) {
		std::ofstream out(completion_file);
		out << R"(#compdef dv

_dv() {
  local completions
  completions=("${(@f)$(dv-binary --tab "${words[@]}")}")

  compadd -S '' -Q -U -V 'Available Options' -- "${completions[@]}"
}
)";
	}

	std::string completion_block;
	if (zshrc_content.find("# Begin Dirvana Zsh completion configuration") == std::string::npos) {
		completion_block = "# Begin Dirvana Zsh completion configuration\n"
			"fpath=(" + completions_dir + " $fpath)\n"
			"\n"
			"zstyle ':completion:*' list-grouped yes\n"
			"zstyle ':completion:*' menu select\n"
			"zstyle ':completion:*' matcher-list '' 'r:|=*'\n"
			"\n"
			"setopt menucomplete\n"
			"setopt autolist\n"
			"\n"
			"autoload -Uz compinit && compinit -u\n"
			"# End Dirvana Zsh completion configuration\n\n";
	}

	// Prepend the completion block so compinit runs before any later completion config
	if (!completion_block.empty()) {
		std::ofstream out(zshrc_path);
		if (!out.is_open()) {
			std::cerr << "Failed to open " << zshrc_path << std::endl;
			return 1;
		}
		out << completion_block << zshrc_content;
		zshrc_content = completion_block + zshrc_content;
	}

	// Append PATH export only if the binary lives in ~/.local/bin and PATH isn't already set
	std::string append_block;
	if (is_local_bin && zshrc_content.find("# Add ~/.local/bin to PATH") == std::string::npos) {
		append_block += "\n# Add ~/.local/bin to PATH\nexport PATH=\"$HOME/.local/bin:$PATH\"\n";
	}

	// Append the dv() function and auto-refresh if not already present
	if (zshrc_content.find("dv-binary --enter dv") == std::string::npos) {
		append_block += R"(
# Dirvana
dv() {
  local cmd
  cmd=$(dv-binary --enter dv "$@")
  if [[ -n "$cmd" ]]; then
    eval "$cmd"
  else
    echo "dv-error: No command found for '$*'"
  fi
}
dv-binary --enter dv refresh &> /dev/null & disown
)";
	}

	if (!append_block.empty()) {
		std::ofstream out(zshrc_path, std::ios::app);
		if (!out.is_open()) {
			std::cerr << "Failed to open " << zshrc_path << std::endl;
			return 1;
		}
		out << append_block;
	}

	std::cerr << "Shell configuration written to " << zshrc_path << std::endl;

	if (!handler.db.build(init_path)) {
		std::cerr << "Failed to build database from " << init_path << std::endl;
		return 1;
	}

	std::cerr << "Database initialized from " << init_path << std::endl;
	std::cerr << "Run: source ~/.zshrc" << std::endl;

	// Emit a no-op on stdout so the dv() shell wrapper's eval succeeds silently
	std::cout << ":" << std::endl;
	return 0;
}

int Handler::Subcommands::handle_add(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for add:
	// None for now

	// Validate the arguments passed to add
	if (commands.size() < 3) {
		std::cerr << "Usage dv add [shortcut] [command]" << std::endl;
		return 1;
	}

	std::string shortcut = commands[1];
	std::string command = commands[2];
	// std::string command = "";
	// for (size_t i = 2; i < commands.size(); i++) {
	// 	if (i > 2) command += " ";
	// 	command += commands[i];
	// }
	// Add the pair to the database
	handler.db.get_shortcuts_table().add_shortcut(shortcut, command);

	std::cout << "echo Shortcut \"" << shortcut << "\" added for command \"" << command << "\"" << std::endl;

	return 0;
}

const std::string Handler::get_init_path() const {
	return db.get_config().get_init_path();
}


int Handler::Subcommands::handle_delete(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for delete:
	// None for now

	if (commands.size() != 2) {
		std::cerr << "Usage dv delete [shortcut]" << std::endl;
		return 1;
	}

	// Validate the arguments passed to delete
	std::string shortcut = commands[1];

	// Delete the shortcut from the database
	handler.db.get_shortcuts_table().delete_shortcut(commands[1]);

	std::cout << "echo Shortcut " << shortcut << " deleted" << std::endl;

	return 0;
}


int Handler::Subcommands::handle_list(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for list:
	// None for now

	// List all shortcuts in the database
	std::vector<std::string> shortcuts = handler.db.get_shortcuts_table().select_all_shortcuts();
	std::string output = "Shortcuts:";
	for (const auto& shortcut : shortcuts)
		output += "\n" + shortcut;
	std::cout << "echo \"" << output << "\"" << std::endl;

	return 0;
}


int Handler::Subcommands::handle_show(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for show: none for now
	
	if (commands.size() != 2) {
		std::cerr << "Usage dv show [shortcut]" << std::endl;
		return 1;
	}

	std::string shortcut = commands[1];

	// Show the shortcut from the database
	std::string command = handler.db.get_shortcuts_table().select_shortcut_command(shortcut);
	if (not command.empty())
		std::cout << "echo \"Shortcut: " << shortcut << " | Command: " << command << "\"" << std::endl;
	else {
		std::cerr << "Shortcut " << shortcut << " not found" << std::endl;
		return 1;
	}

	return 0;
}
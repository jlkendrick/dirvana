#include "Handler.h"

Handler::Handler(Database& db, const std::string& version) : db(db), version(version) {}

int Handler::handle_tab(int argc, char* argv[]) {
	// Need at least 4 arguments: dv_binary, --tab, dv, partial_path
	if (argc < 4) {
		std::cerr << "Tab completion requires at least a partial path" << std::endl;
		return 1;
	}
	
	// Last argument is the partial path
	std::string partial = argv[argc - 1];
	
	// Get matches for the partial path
	std::vector<std::string> matches = db.query(partial);
	
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

		// Check if a subcommand was passed
		// We treat the first (non-flag) argument as a potential subcommand
		std::string subcommand = commands[0];
		if (subcommand == "build" or subcommand == "rebuild") {
			return Subcommands::handle_re_build(*this, commands, flags);
		} else if (subcommand == "refresh") {
			return Subcommands::handle_refresh(*this, commands, flags);
		} else if (subcommand == "install") {
			return Subcommands::handle_install(*this, commands, flags);
		} else if (subcommand == "add") {
			return Subcommands::handle_add(*this, commands, flags);
		}
		
		// Normal case: a path was passed
		// Last argument (or arg passed to --) is the path to complete
		std::string path = !commands.empty() ? commands.back() : ArgParsing::get_flag_value(flags, "[bypass]");
		// Check if path is full path or partial
		if (path.find('/') == std::string::npos and path.find('~') == std::string::npos and path.find('/') == std::string::npos) {
			// Partial path, need to complete
			std::vector<std::string> matches = db.query(path);
			if (matches.empty()) {
				// 'cd' to the path if no matches found for entries like "~", "..", etc.
				std::cout << "cd " << path << std::endl;
				return 0;
			}
			// Use the first match
			path = matches[0];
		}
		
		// Now we need to assemble the final command to output.
		// Arguments look something like: dv-binary [call_type] dv [...] [path]
		std::string prefix = "";
		if (commands.size() > 1) {
			// Build the prefix from all arguments except the last one (and any "--" delimiters)
			for (size_t i = 0; i < commands.size() - 1; i++) {
				if (i > 0) prefix += " ";
				prefix += commands[i] != "--" ? commands[i] : "";
			}
		}

		// Update the database with the accessed path
		db.access(path);

		// Check if the path is a shortcut by checking for the :{dv-shortcut} suffix
		if (path.find(":{dv-shortcut}") != std::string::npos) {
			std::string shortcut = path.substr(0, path.find(":{dv-shortcut}"));
			std::cout << shortcut << std::endl;
			return 0;
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
				
	if (handler.db.build(init_path)) {
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

int Handler::Subcommands::handle_add(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags) {
	// Relevant flags for add:
	// None for now

	// Validate the arguments passed to add
	if (commands.size() < 3 or commands.size() > 4) {
		std::cerr << "Usage dv add [shortcut] [command]" << std::endl;
		return 1;
	}

	std::string shortcut = commands[1];
	std::string command = commands[2];
	// Add the pair to the database
	handler.db.add_shortcut(shortcut, command);

	return 0;
}

const std::string Handler::get_init_path() const {
	return db.get_config().get_init_path();
}
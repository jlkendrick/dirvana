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
			else if (first_token == "add")
				return Subcommands::handle_add(*this, commands, flags);
		}

		// If we are here, need to handle a shortcut or a path. We prioritize shortcuts over paths

		// Check if the path is a shortcut by checking for the :{dv-shortcut} suffix
		std::vector<std::string> matches = db.query(first_token, true);
		if (!matches.empty()) {
			for (const auto& match : matches) {
				// Check if the match is a shortcut. If so, execute the shortcut.
				if (match.find(":{dv-shortcut}") != std::string::npos) {
					std::string command = match.substr(0, match.find(":{dv-shortcut}"));

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
						std::vector<std::string> matches = db.query(last_token);
						if (!matches.empty())
							last_token = matches[0];
					}

					// Add the last token to the arguments
					args += " " + last_token;

					// Execute the shortcut
					std::cout << command << args << std::endl;

					// Update the database with the accessed path
					db.access(match);

					return 0;
				}
			}
		}

		// If we are here, we need to handle a path
		
		// Last token (or arg passed to --) is the path to complete
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

		// Update the database with the accessed path
		db.access(path);
		
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

	std::cout << "echo Shortcut " << shortcut << " added for command " << command << std::endl;

	return 0;
}

const std::string Handler::get_init_path() const {
	return db.get_config().get_init_path();
}
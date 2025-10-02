#include "Database.h"
#include "Helpers.h"

#include <fstream>


void write_log(const std::string& message) {
	std::ofstream log_file("/Users/jameskendrick/Code/Projects/dirvana/build/logs.txt", std::ios::app);
	if (log_file.is_open()) {
		log_file << message << std::endl;
		log_file.close();
	} else
		std::cerr << "Unable to open log file" << std::endl;
}

static std::string version = "1.0.1";

int main(int argc, char* argv[]) {
	auto start = std::chrono::high_resolution_clock::now();
	// Keeping this here for future reference if we want to log time taken for an operation
	// Log the time taken for the operation
	// auto end = std::chrono::high_resolution_clock::now();
	// auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	// std::string log_message = "xxx " + std::to_string(duration.count()) + " us for partial: " + partial;
	// write_log(log_message);

	// Initialize the database
	Config config;
	Database db(config);

	// Need at least 2 arguments: program name and a flag
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [--tab|--enter] dv [command] [path]" << std::endl;
		return 1;
	}

	auto [
		valid,
		call_type,
		commands, 
		flags
	] = ArgParsing::process_args(argc, argv);
	if (!valid)
		// Error message already printed in process_args
		return 1;
	
	// Handle tab completion
	if (call_type == "--tab") {
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
	
	// Handle enter key press
	else if (call_type == "--enter") {
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
		// We only allow subcommands when there is only one argument after "dv"
		if (commands.size() == 1) {
			std::string subcommand = commands[0];

			// Flag parsing
			std::string init_path = ArgParsing::get_flag_value(flags, "root", config.get_init_path());
			if (subcommand == "build" or subcommand == "rebuild") {
				if (db.build(init_path)) {
					std::cout << "echo Build from " << init_path << " complete" << std::endl;
					return 0;
				} else
					return 1;
			} else if (subcommand == "refresh") {
				if (db.refresh(init_path)) {
					std::cout << "echo Refresh from " << init_path << " complete" << std::endl;
					return 0;
				} else
					return 1;
			}
		}
		
		// Last argument is the path to complete
		std::string path = commands.back();
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
	
	// Invalid flag
	else {
		std::cerr << "Invalid flag: " << call_type << std::endl;
		std::cerr << "Usage: " << argv[0] << " [-tab|-enter] dv [command] [path]" << std::endl;
		return 1;
	}
}
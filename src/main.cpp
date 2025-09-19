#include "Database.h"

#include <fstream>


void write_log(const std::string& message) {
	std::ofstream log_file("/Users/jameskendrick/Code/Projects/dirvana/build/logs.txt", std::ios::app);
	if (log_file.is_open()) {
		log_file << message << std::endl;
		log_file.close();
	} else {
		std::cerr << "Unable to open log file" << std::endl;
	}
}

int main(int argc, char* argv[]) {
	auto start = std::chrono::high_resolution_clock::now();
	// Keeping this here for future reference if we want to log time taken for operations
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

	std::string call_type = argv[1];
	
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
		// Need at least 3 arguments: program_name, -enter, dv
		if (argc < 3 || std::string(argv[2]) != "dv") {
			std::cerr << "Enter handler requires 'dv' as the first argument" << std::endl;
			return 1;
		}

		// If -enter was called with no arguments, that is the eqivalent of "cd"
		// where we want to cd to home dir
		if (argc == 3 ) {
			std::cout << "cd ~" << std::endl;
			return 0;
		}

		// Check if a command was passed
		std::string last_arg = argv[argc - 1];
		std::string second_last_arg = (argc > 3) ? argv[argc - 2] : "";
		// We use "--" as a delimiter to separate commands from paths
		if (second_last_arg != "--") {
			if (last_arg == "rebuild") {
				db.build();
				std::cout << "echo Rebuild complete" << std::endl;
				return 0;
			} else if (last_arg == "refresh") {
				db.refresh();
				std::cout << "echo Refresh complete" << std::endl;
				return 0;
			}
		}

		// At this point we know we need to handle a path, but are unsure if it needs
		// to be completed or not
		std::string path = last_arg;
		std::string result;

		// Check if path is full path or partial
		if (path.find('/') != std::string::npos || path.find('~') == 0) {
			result = path;
		} else {
			// Partial path, need to complete
			std::vector<std::string> matches = db.query(path);
			if (matches.empty()) {
				// 'cd' to the path if no matches found for entries like "~", "..", etc.
				std::cout << "cd " << path << std::endl;
				return 0;
			}
			// Use the first match
			result = matches[0];
		}
		
		// Now we need to assemble the final command to output.
		// Arguments look something like: dv-binary [call_type] dv [...] [path]
		std::string command = "";
		if (argc >= 4) {
			// Build the command from all arguments except the last one (and any "--" delimiters)
			for (int i = 3; i < argc - 1; i++) {
				if (i > 3) command += " ";
				command += std::string(argv[i]) == "--" ? "" : argv[i];
			}
		} else {
			std::cerr << "No path specified" << std::endl;
			return 1;
		}

		// Update the database with the accessed path
		db.access(result);
			
		// Decide how to output based on presence of command
		if (command.empty()) {
			// No command, just 'cd' to the path
			std::cout << "cd " << result << std::endl;
		} else {
			// Execute the command with the path
			std::cout << command << " " << result << std::endl;
		}

		return 0;
	}
	
	// Invalid flag
	else {
		std::cerr << "Invalid flag: " << call_type << std::endl;
		std::cerr << "Usage: " << argv[0] << " [-tab|-enter] dv [command] [path]" << std::endl;
		return 1;
	}
}
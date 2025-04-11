#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

void rebuild() {
	// To build the DirectoryCompleter, we need to create it and save it to a file
	DirectoryCompleter completer(DCArgs{ .build= true });
	completer.save();
}

void refresh() {
	// To refresh the DirectoryCompleter, we need to load it, refresh it by scanning the (potientially) new directories, and save it to a file
	DirectoryCompleter completer(DCArgs{ .build= false, .refresh= true });
	completer.save();
}

void update(const std::string& path, DirectoryCompleter& completer) {
	// To update the DirectoryCompleter, we need to access the path, and re-save it
	completer.access(path);
	completer.save();
}

std::vector<std::string> query(const std::string& partial) {
	// To query the DirectoryCompleter, we need to load it and get all matches for the partial path
	// Note: This version is used for the tab completion, we don't update positions until the user actually uses the path
	DirectoryCompleter completer(DCArgs{ .build= false });
	return completer.get_matches(partial);
}

std::vector<std::string> query(const std::string& partial, const DirectoryCompleter& completer) {
	// To query the DirectoryCompleter, we need to query the completer and get all matches for the partial path
	// Note: This version is used for the enter key press, where we have already constructed the completer
	return completer.get_matches(partial);
}


int main(int argc, char* argv[]) {
	// Write the arguments to a file for debugging
	// ofstream io_file("/Users/jameskendrick/Code/Projects/dirvana/build/debug_io.txt", ios::app);
	// for (int i = 0; i < argc; i++)
		// io_file << argv[i] << " ";
	// io_file << std::endl;

	// Need at least 2 arguments: program name and a flag
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [-tab|-enter] dv [command] [path]" << std::endl;
		return 1;
	}

	std::string call_type = argv[1];
	
	// Handle tab completion
	if (call_type == "-tab") {
		// Need at least 4 arguments: program_name, -tab, dv, partial_path
		if (argc < 4) {
			std::cerr << "Tab completion requires at least a partial path" << std::endl;
			return 1;
		}
		
		// Last argument is the partial path
		std::string partial = argv[argc - 1];
		
		// Get matches for the partial path
		std::vector<std::string> matches = query(partial);
		
		// Check if there are commands between "dv" and the partial path
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
	else if (call_type == "-enter") {
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

		// Check if a flag was passed
		if (std::string(argv[3]).find("--") == 0) {
			if (std::string(argv[3]) == "--cmd") {
				if (argc < 5) {
					std::cerr << "No command specified after --cmd flag" << std::endl;
					return 1;
				}
				std::string command = argv[4];

				if (command == "rebuild") {
					rebuild();
					std::cout << "echo Rebuild complete" << std::endl;
					return 0;
				} else if (command == "refresh") {
					refresh();
					std::cout << "echo Refresh complete" << std::endl;
					return 0;
				} else {
					std::cerr << "Invalid command: " << command << std::endl;
					return 1;
				}
			} 
			
			// We will add more flags here in the future
			
			else {
				std::cerr << "Invalid flag: " << argv[3] << std::endl;
				return 1;
			}
		}
			
		// Now we need to handle cases i, ii, iii, and iiii
		std::string current_command = "";
		std::string path = "";
		
		// arguments look something like: dv-binary [call_type] dv [path] [...]
		// Step 4: Check number of arguments after "dv"
		if (argc == 4) {
			// Case i or ii: Only one argument after "dv"
			path = argv[3];
		} else if (argc > 4) {
			// Case iii or iiii: Multiple arguments after "dv"
			// Build the command from all arguments except the last one
			for (int i = 3; i < argc - 1; i++) {
				if (i > 3) current_command += " ";
				current_command += argv[i];
			}
			// The last argument is the path
			path = argv[argc - 1];
		} else {
			std::cerr << "No path specified" << std::endl;
			return 1;
		}
		
		// Step 5: Check if path is full path or partial
		std::string result;

		// Create a single DirectoryCompleter instance to avoid repeated construction
		DirectoryCompleter completer(DCArgs{ .build= false });

		if (path.find('/') != std::string::npos || path.find('~') == 0) {
			// Case i or iii: Full path
			result = path;
		} else {
			// Case ii or iiii: Partial path
			std::vector<std::string> matches = query(path, completer);
			if (matches.empty()) {
				// 'cd' to the path if no matches found for entries like "~", "..", etc.
				std::cout << "cd " << path << std::endl;
				return 0;
			}
			// Use the first match
			result = matches[0];
		}

		// Access the path in the DirectoryCompleter (will extract the partial)
		completer.access(result);
		completer.save(); // Save the updated DirectoryCompleter
			
		// Step 6: Generate the appropriate command
		if (current_command.empty()) {
			// Case i or ii: Just cd to the path
			std::cout << "cd " << result << std::endl;
		} else {
			// Case iii or iiii: Execute the command with the path
			std::cout << current_command << " " << result << std::endl;
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
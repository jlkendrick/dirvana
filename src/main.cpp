#include "DirectoryCompleter.h"

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

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

void update(const string& path, DirectoryCompleter& completer) {
	// To update the DirectoryCompleter, we need to access the path, and re-save it
	completer.access(path);
	completer.save();
}

vector<string> query(const string& partial) {
	// To query the DirectoryCompleter, we need to load it and get all matches for the partial path
	// Note: This version is used for the tab completion, we don't update positions until the user actually uses the path
	DirectoryCompleter completer(DCArgs{ .build= false });
	return completer.get_all_matches(partial);
}

vector<string> query(const string& partial, const DirectoryCompleter& completer) {
	// To query the DirectoryCompleter, we need to query the completer and get all matches for the partial path
	// Note: This version is used for the enter key press, where we have already constructed the completer
	return completer.get_all_matches(partial);
}

int main(int argc, char* argv[]) {
	// Write the arguments to a file for debugging
	// ofstream io_file("/Users/jameskendrick/Code/Projects/dirvana/build/debug_io.txt", ios::app);
	// for (int i = 0; i < argc; i++)
		// io_file << argv[i] << " ";
	// io_file << endl;

	// Need at least 2 arguments: program name and a flag
	if (argc < 2) {
		cerr << "Usage: " << argv[0] << " [-tab|-enter] dv [command] [path]" << endl;
		return 1;
	}

	string flag = argv[1];
	
	// Handle tab completion
	if (flag == "-tab") {
		// Need at least 4 arguments: program_name, -tab, dv, partial_path
		if (argc < 4) {
			cerr << "Tab completion requires at least a partial path" << endl;
			return 1;
		}
		
		// Last argument is the partial path
		string partial = argv[argc - 1];
		
		// Get matches for the partial path
		vector<string> matches = query(partial);
		
		// Check if there are commands between "dv" and the partial path
		string prefix = "";
		for (int i = 3; i < argc - 1; i++) {
			if (i > 3) prefix += " ";
			prefix += argv[i];
		}
		
		// Print the matches with appropriate prefixes for zsh completion
		for (const auto& match : matches) {
			if (!prefix.empty()) {
				// cout << prefix << " ";
				// io_file << prefix << " ";
			}
			cout << match << endl;
			// io_file << match << endl;
		}

		// io_file << "-----------------------------" << endl;
		// io_file.close();
		
		return 0;
	}
	
	// Handle enter key press
	else if (flag == "-enter") {
		// Need at least 3 arguments: program_name, -enter, dv
		if (argc < 3 || string(argv[2]) != "dv") {
			cerr << "Enter handler requires 'dv' as the first argument" << endl;
			return 1;
		}
			
		// Check for bypass flag
		bool bypass = false;
		int bypass_index = 0;
		for (int i = 3; i < argc; i++) {
			if (string(argv[i]) == "--") {
				bypass = true;
				bypass_index = i;
				break;
			}
		}
			
		// Handle rebuild command
		if (!bypass && argc >= 4 && string(argv[3]) == "rebuild") {
			rebuild();
			// io_file << "-------------------------------" << endl;
			// io_file.close();
			cout << "echo Rebuild complete" << endl;
			return 0;
		}
			
		// Handle refresh command
		if (!bypass && argc >= 4 && string(argv[3]) == "refresh") {
			refresh();
			// io_file << "-------------------------------" << endl;
			// io_file.close();
			cout << "echo Refresh complete" << endl;
			return 0;
		}
			
		// Now we need to handle cases i, ii, iii, and iiii
		string current_command = "";
		string path = "";
			
		// Step 4: Check number of arguments after "dv", accouning for bypass
		if (bypass) {
			// Handle the case where the bypass flag is present
			if (bypass_index == argc - 1) {
				// If the bypass flag is the last argument, there's no path
				cerr << "No path specified after bypass flag" << endl;
				return 1;
			}
		
			// Only one argument after the bypass flag (the path)
			if (bypass_index == argc - 2) {
				path = argv[argc - 1];
			} else {
				// Multiple arguments after the bypass flag
				// Collect all arguments after the bypass flag into the path
				path = "";
				for (int i = bypass_index + 1; i < argc; i++) {
					if (i > bypass_index + 1) path += " ";
						path += argv[i];
				}
			}
		
			// Collect any commands before the bypass flag
			for (int i = 3; i < bypass_index; i++) {
				if (i > 3) current_command += " ";
				current_command += argv[i];
			}

		} else {
			// Handle the case where the bypass flag is not present
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
				cerr << "No path specified" << endl;
				return 1;
			}
		}
		
		
		// Step 5: Check if path is full path or partial
		string result;

		// Create a single DirectoryCompleter instance to avoid repeated construction
		DirectoryCompleter completer(DCArgs{ .build= false });

		if (path.find('/') != string::npos || path.find('~') == 0) {
			// Case i or iii: Full path
			result = path;
		} else {
			// Case ii or iiii: Partial path
			vector<string> matches = query(path, completer);
			if (matches.empty()) {
				// 'cd' to the path if no matches found for entries like "~", "..", etc.
				cout << "cd " << path << endl;
				// io_file << "cd " << path << endl;
				// io_file << "-----------------------------" << endl;
				// io_file.close();
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
			cout << "cd " << result << endl;
			// io_file << "cd " << result << endl;
			// io_file << "-----------------------------" << endl;
		} else {
			// Case iii or iiii: Execute the command with the path
			cout << current_command << " " << result << endl;
			// io_file << current_command << " " << result << endl;
			// io_file << "-----------------------------" << endl;
		}
		
		// io_file.close();

		return 0;
	}
	
	// Invalid flag
	else {
		cerr << "Invalid flag: " << flag << endl;
		cerr << "Usage: " << argv[0] << " [-tab|-enter] dv [command] [path]" << endl;
		return 1;
	}
}
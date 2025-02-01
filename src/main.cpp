#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

using namespace std;

void build() {
	// Build the DirectoryCompleter and save it to a file (use 'build=true' constructor)
	DirectoryCompleter completer(DCArgs{ .build= true });
	completer.save();
}

void update(const string& path) {
	// Load the DirectoryCompleter (use 'build=false' constructor)
	DirectoryCompleter completer(DCArgs{ .build= false });

	// Update the accessed entry
	completer.access(path);

	// Save the DirectoryCompleter to a file
	completer.save();
}

void query(const string& partial) {
	// Load the DirectoryCompleter (use 'build=false' constructor)
	DirectoryCompleter completer(DCArgs{ .build= false });

	// Get the matches for the partial path
	auto matches = completer.get_all_matches(partial);

	// Print the matches
	for (const auto& match : matches) {
		cout << match << endl;
	}
}


int main(int argc, char* argv[]) {
	if (argc < 2)
		// We expect at least 1 argument, the command
		return 0;

	string command = argv[1];
	if (command == "build") {
		// Build (or re-build) the DirectoryCompleter and save it to a file
		build();
		return 0;
	} else if (command == "update") {
		// Load the DirectoryCompleter, update the accessed entry, and save it to a file
		// First, we need to check if we have an argument for the path
		if (argc < 3)
			// We expect at least 2 arguments, the command and the path
			return 0;
		
		update(argv[2]);
		return 0;
	}
	
	string partial;
	if (command == "--") {
		// If the command is "--", we expect the partial path as the next argument
		if (argc < 3)
			// We expect at least 2 arguments, the command and the partial path
			return 0;
		partial = argv[2];
	} else {
		// Otherwise, we expect the partial path as the last argument
		partial = command;
	}

	// If here, treat this as 'query' mode.
	query(partial);
	return 0;
}
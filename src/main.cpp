#include "DirectoryCompleter.h"

#include <thread>
#include <iostream>
#include <string>

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

void update(const string& path) {
	// To update the DirectoryCompleter, we need to load it, access the path, and re-save it
	DirectoryCompleter completer(DCArgs{ .build= false });
	completer.access(path);
	completer.save();
}

vector<string> query(const string& partial) {
	// To query the DirectoryCompleter, we need to load it and print the matches
	DirectoryCompleter completer(DCArgs{ .build= false });
	return completer.get_all_matches(partial);
}


int main(int argc, char* argv[]) {



	if (argc < 3) {
		// We expect at least 2 arguments, the program name, the flag, and some command or path
		cout << "Usage: dv <command> [args]" << endl;
		return 0;
	}

	string command = argv[1];
	if (command == "rebuild") {
		// Rebuild the DirectoryCompleter from scratch and save it to a file, resetting order
		rebuild();
		return 0;
	}

	if (command == "refresh") {
		// Rebuild the DirectoryCompleter while preserving the existing data and save it to a file (way to account for added/removed directories)
		refresh();
		return 0;
	}

	if (command == "update") {
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
	} else
		// Otherwise, we expect the partial path as the last argument
		partial = command;

	// If here, treat this as 'query' mode.
	auto matches = query(partial);
	for (const auto& match : matches) {
		cout << match << endl;
	}
	
	return 0;
}
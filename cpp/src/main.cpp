#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 3)
		// We expect at least 2 arguments: partial path and root directory
		return 0;

	string partial_path = argv[1];
	string root = argv[2];

	// Create the directory completer
	DirectoryCompleter completer(root);

	// Get the list of completions
	auto completions = completer.get_all_matches(partial_path);

	// Print the completions
	for (const auto& completion : completions) {
		cout << completion << endl;
	}

	return 0;
}
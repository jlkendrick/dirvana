#include "DirectoryCompleter.h"

#include <iostream>

using namespace std;

int main() {
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp";
	cout << "Initializing Completer with root directory: " << root << endl;

	DirectoryCompleter completer(root);
	cout << "Completer initialized with " << completer.get_size() << " directories" << endl;
} 
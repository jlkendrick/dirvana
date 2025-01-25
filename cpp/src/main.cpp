#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
	
	string root = "/Users/jameskendrick/Code/Projects/dirvana/cpp/mockfs";
	cout << "Initializing Completer with root directory: " << root << endl;

	DirectoryCompleter completer(root);
	cout << "Completer initialized with " << completer.get_size() << " directories" << endl;
} 
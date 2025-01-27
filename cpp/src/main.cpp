#include "DirectoryCompleter.h"

#include <iostream>
#include <string>

using namespace std;

int main(int argc, char* argv[]) {
	
	string root = "/Users/jameskendrick/Code/Projects/wava/SOAPAI";
	cout << "Initializing Completer with root directory: " << root << endl;

	DirectoryCompleter completer(root);
	cout << "Completer initialized with " << completer.get_size() << " directories" << endl;

	cout << completer.complete("node_modules").size() << endl;
}
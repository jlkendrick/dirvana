#ifndef HANDLER_H
#define HANDLER_H

#include "Database.h"


// Class to handle the main logic of the program
class Handler {
public:
	Handler(Database& db);

	int handle_tab(std::string partial_path);
	int handle_enter(std::vector<std::string>& commands, std::vector<Flag>& flags);

private:
	Database& db;
};

#endif // HANDLER_H
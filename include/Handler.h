#ifndef HANDLER_H
#define HANDLER_H

#include "Database.h"

// Class to handle the main logic of the program
class Handler {
public:
	Handler(Database& db, const std::string& version = "1.0.1");

	int handle_tab(int argc, char* argv[]);
	int handle_enter(std::vector<std::string>& commands, std::vector<Flag>& flags);

	struct Subcommands {
		static int handle_re_build(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags);
		static int handle_refresh(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags);
		static int handle_install(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags);
		static int handle_add(Handler& handler, std::vector<std::string>& commands, std::vector<Flag>& flags);
	};

	
	const std::string get_init_path() const;

private:
	Database& db;
	const std::string version;
};

#endif // HANDLER_H
#include "Handler.h"

Handler::Handler(Database& db) : db(db) {}

int Handler::handle_tab(std::string partial_path) {
	return 0;
}

int Handler::handle_enter(std::vector<std::string>& commands, std::vector<Flag>& flags) {
	return 0;
}
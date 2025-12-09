#include "Database.h"
#include "Handler.h"
#include "Helpers.h"

#include <fstream>


void write_log(const std::string& message) {
	std::ofstream log_file("/Users/jameskendrick/Code/Projects/dirvana/build/logs.txt", std::ios::app);
	if (log_file.is_open()) {
		log_file << message << std::endl;
		log_file.close();
	} else
		std::cerr << "Unable to open log file" << std::endl;
}

int main(int argc, char* argv[]) {
	// Initialize the database
	Config config;
	Database db(config);
	Handler handler(db, "1.0.1");

	// Need at least 2 arguments: program name and a flag
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [--tab|--enter] dv [command] [path]" << std::endl;
		return 1;
	}

	auto [
		valid,
		call_type,
		commands, 
		flags
	] = ArgParsing::process_args(argc, argv);
	if (!valid)
		// Error message already printed in process_args
		return 1;
	
	// Handle tab completion
	if (call_type == "--tab") {
		return handler.handle_tab(argc, argv);
	}

	// Handle enter key press
	else if (call_type == "--enter") {
		return handler.handle_enter(commands, flags);
	}
	
	else {
		std::cerr << "Invalid flag: " << call_type << std::endl;
		std::cerr << "Usage: " << argv[0] << " [-tab|-enter] dv [command] [path]" << std::endl;
		return 1;
	}
}
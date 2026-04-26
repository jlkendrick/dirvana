#include "Database.h"
#include "Handler.h"
#include "Helpers.h"

#ifndef DIRVANA_VERSION
#define DIRVANA_VERSION "dev"
#endif

int main(int argc, char* argv[]) {

	// Initialize the database
	Config config;
	Database db(config);
	Handler handler(db, DIRVANA_VERSION);

	// Need at least 2 arguments: program name and a flag
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " [--tab|--enter] dv [command] [path]" << std::endl;
		return 1;
	}

	std::string call_type = argc > 1 ? argv[1] : "";

	// Handle tab completion
	if (call_type == "--tab") {
		return handler.handle_tab(argc, argv);
	}

	// Direct subcommand invocation (e.g. `dv-binary init`) — used before the dv() shell function
	// is available. Synthesize the `--enter dv` prefix so process_args sees the expected structure.
	std::vector<std::string> wrapped_storage;
	std::vector<char*> wrapped_argv;
	int effective_argc = argc;
	char** effective_argv = argv;
	if (call_type != "--enter") {
		wrapped_storage.reserve(argc + 2);
		wrapped_storage.push_back(argv[0]);
		wrapped_storage.push_back("--enter");
		wrapped_storage.push_back("dv");
		for (int i = 1; i < argc; i++) wrapped_storage.push_back(argv[i]);
		for (auto& s : wrapped_storage) wrapped_argv.push_back(s.data());
		effective_argc = wrapped_argv.size();
		effective_argv = wrapped_argv.data();
	}

	auto [valid, commands, flags] = ArgParsing::process_args(effective_argc, effective_argv);
	if (!valid)
		return 1;

	return handler.handle_enter(commands, flags);
}
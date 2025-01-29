#ifndef TERMINALHANDLER_H
#define TERMINALHANDLER_H

#include "DirectoryCompleter.h"
#include "DLLTraverser.h"

#include <termios.h>
#include <unistd.h>

#include <memory>
#include <string>

class TerminalHandler {
public:
	TerminalHandler(DirectoryCompleter& completer) : completer(completer) {
		enable_raw_mode();
	};

	~TerminalHandler() {
		disable_raw_mode();
	};

	void run();

private:
	DirectoryCompleter& completer;
	std::unique_ptr<DLLTraverser> traverser = nullptr;
	// True if we are currently cycling through the directory completions
	bool using_traverser = false;

	std::string current_command;
	std::string partial_input;

	struct termios original_termios;
	
	void enable_raw_mode();
	void disable_raw_mode();

	void handle_tab();
	void handle_leftarrow();
	void handle_rightarrow();
	void handle_enter();
	void handle_backspace();
	void handle_normal_char(char c);
	void handle_escape_sequence();

	void refresh_line(const std::string& line);
};

#endif // TERMINALHANDLER_H
#include "TerminalHandler.h"
#include "DirectoryCompleter.h"

#include <iostream>

void TerminalHandler::run() {
	current_command = "cd";
	refresh_line(current_command);

	char c;
	while (true) {
		if (read(STDIN_FILENO, &c, 1) <= 0) {
			break;
		}

		if (c == '\n' || c == '\r') {
			handle_enter();
		} else if (c == '\t') {
			handle_tab();
		} else if (c == 127 || c == 8) {
			handle_backspace();
		} else if (c == 27) {
			handle_escape_sequence();
		} else {
			handle_normal_char(c);
		}
	}
}

void TerminalHandler::handle_tab() {
	// Extract the directory name from the current command
	if (current_command.size() > 3)
		partial_input = current_command.substr(3);
	else
		partial_input = "";
	
	if (completer.has_matches(partial_input)) {
		// Create a new traverser if one doesn't exist
		traverser = std::make_unique<DLLTraverser>(completer.get_list_for(partial_input));
		using_traverser = true;

		// Set the current_command to the first match
		current_command = "cd " + traverser->current();
		// Refresh the line
		refresh_line(current_command);
	} else {
		return;
	}
}

void TerminalHandler::handle_leftarrow() {
	if (using_traverser) {
		traverser->move_prev();
		current_command = "cd " + traverser->current();
		refresh_line(current_command);
	}
}

void TerminalHandler::handle_rightarrow() {
	if (using_traverser) {
		traverser->move_next();
		current_command = "cd " + traverser->current();
		refresh_line(current_command);
	}
}

void TerminalHandler::handle_enter() {
	// Clear the traverser
	traverser = nullptr;
	using_traverser = false;

	// Execute the command
	const char* cmd = current_command.c_str();
	if (system(cmd) == -1) {
		std::cerr << "Error executing command: " << cmd << std::endl;
	}
	
	// Clear the current command
	current_command = "";
	refresh_line(current_command);
}

void TerminalHandler::handle_backspace() {
	// Discard the traverser
	if (using_traverser) {
		traverser = nullptr;
		using_traverser = false;
	}

	// Remove the last character from the current command
	if (!current_command.empty())
		current_command.pop_back();
	refresh_line(current_command);
}

void TerminalHandler::handle_normal_char(char c) {
	// Discard the traverser
	if (using_traverser) {
		traverser = nullptr;
		using_traverser = false;
	}

	// Add the character to the current command
	current_command += c;
	refresh_line(current_command);
}

void TerminalHandler::handle_escape_sequence() {
	// We have already read '\x1b'
	char seq[2];
	if (read(STDIN_FILENO, &seq[0], 1) < 1)
		return;
	if (read(STDIN_FILENO, &seq[1], 1) < 1)

	if (seq[0] == '[') {
		switch (seq[1]) {
			case 'D':
				handle_leftarrow();
				break;
			case 'C':
				handle_rightarrow();
				break;
		}
	}
}
		
void TerminalHandler::refresh_line(const std::string& line) {
	// Move the cursor to the beginning of the line
	write(STDOUT_FILENO, "\r", 1);
	// Clear the line
	write(STDOUT_FILENO, "\x1b[K", 3);

	// Print the command
	write(STDOUT_FILENO, line.c_str(), line.size());
}
	

void TerminalHandler::enable_raw_mode() {
	tcgetattr(STDIN_FILENO, &original_termios);
	struct termios raw = original_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void TerminalHandler::disable_raw_mode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
}
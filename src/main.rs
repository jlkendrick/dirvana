mod utils;
use crossterm::{
	event::{self, Event, KeyCode},
	terminal::{disable_raw_mode, enable_raw_mode},
};
use std::{io::{self, Write}, result::Result};
use utils::file_ops; // Assuming your collect_directories is here

fn main() -> Result<(), Box<dyn std::error::Error>> {
	// Get starting directory from user
	print!("Enter the starting directory: ");
	io::stdout().flush().unwrap();

	let mut input = String::new();
	io::stdin().read_line(&mut input).unwrap();
	let start_dir = input.trim();

	// Collect directories
	let directories = file_ops::collect_directories(start_dir);

	// Enable raw mode for real-time input
	enable_raw_mode()?;

	let mut filter = String::new();

	println!("Type to filter directories (Press ESC to exit):");

	loop {
		// Check for keyboard events
		if event::poll(std::time::Duration::from_millis(500))? {
			if let Event::Key(key_event) = event::read()? {
				match key_event.code {
					KeyCode::Char(c) => {
						filter.push(c); // Add character to filter
						print_filter_results(&filter, &directories);
					}
					KeyCode::Backspace => {
						filter.pop(); // Remove last character
						print_filter_results(&filter, &directories);
					}
					KeyCode::Enter => {
						println!("\nFinal Filter: {}", filter);
						break;
					}
					KeyCode::Esc => {
						println!("\nExiting...");
						break;
					}
					_ => {}
				}
			}
		}
	}

	// Disable raw mode after use
	disable_raw_mode()?;
	Ok(())
}

fn print_filter_results(filter: &str, directories: &[String]) {
	const MAX_RESULTS: usize = 10;

	// Clear previous line
	print!("\x1B[2K\r");

	// Print current filter and matching results
	println!("Filter: {} | Matching directories: ", filter);
	let matching_dirs = directories.iter().filter(|d| d.contains(filter));
	for dir in matching_dirs.clone().take(MAX_RESULTS) {
		print!("{}, ", dir);
	}

	if matching_dirs.count() > MAX_RESULTS {
		println!("...and more");
	}

	io::stdout().flush().unwrap();
}
mod utils;

use utils::file_ops;
use utils::inv_index;

use crossterm::{
	event::{self, Event, KeyCode},
	terminal::{disable_raw_mode, enable_raw_mode},
};
use std::{io::{self, Write}, result::Result};


fn main() -> Result<(), Box<dyn std::error::Error>> {
	// Get starting directory from user
	print!("Enter the starting directory: ");
	io::stdout().flush().unwrap();

	let mut input = String::new();
	io::stdin().read_line(&mut input).unwrap();
	let start_dir = input.trim();

	// Collect directories
	let directories = file_ops::collect_directories(start_dir);

	// Build inverted index
	let index = inv_index::InvertedIndex::build(&directories);

	// Enable raw mode for real-time input
	enable_raw_mode()?;

	let mut query = String::new();

	println!("Type to filter directories (Press ESC to exit):");

	loop {
		// Check for keyboard events
		if event::poll(std::time::Duration::from_millis(500))? {
			if let Event::Key(key_event) = event::read()? {
				match key_event.code {
					KeyCode::Char(c) => {
						query.push(c); // Add character to filter
						print_current_query(&query);
					}
					KeyCode::Backspace => {
						query.pop(); // Remove last character
						print_current_query(&query);
					}
					KeyCode::Enter => {
						query = query.trim().to_string();
						println!("\n Query: {}", query);
						print_query_results(&query, &index);
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

	disable_raw_mode()?;
	Ok(())
}

fn print_current_query(query: &str) {
	// Clear previous line
	print!("\x1B[2K\r");

	// Print current filter
	print!("Filter: {}", query);
	io::stdout().flush().unwrap();
}

fn print_query_results(query: &str, index: &inv_index::InvertedIndex) {
	const MAX_RESULTS: usize = 10;

	// Clear previous line
	print!("\x1B[2K\r");

	// Print current filter and matching results
	println!("Filter: {} | Matching directories: ", query);
	let matching_dirs = index.search(query);
	for dir in matching_dirs.unwrap_or(&Vec::new()).iter().take(MAX_RESULTS) {
		println!("{}", dir);
	}

	// Check if there are more results
	if matching_dirs.unwrap_or(&Vec::new()).len() > MAX_RESULTS {
		println!("... and {} more", matching_dirs.unwrap().len() - MAX_RESULTS);
	}

	io::stdout().flush().unwrap();
}
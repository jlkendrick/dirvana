use std::collections::{HashSet, VecDeque};
use std::path::Path;
use std::fs;

// Collects all the directories starting from a given root
pub fn collect_directories(root: &Path) -> Vec<String> {
	let mut directories = Vec::new();
	let mut queue = VecDeque::new();
	let mut visited = HashSet::new();

	queue.push_back(root.to_path_buf());
	visited.insert(root.to_path_buf());

	// While there are still directories to visit
	while let Some(current_dir) = queue.pop_front() {

		// If we have already visited this directory, skip it
		if !visited.insert(current_dir.clone()) {
			continue;
		}

		// Get an iterator over the entries in the current directory
		if let Ok(entries) = fs::read_dir(&current_dir) {
			for entry in entries.flatten() {
				let path = entry.path();

				println!("Path: {:?}", path);

				// If the entry is a directory, add it to the queue
				if path.is_dir() {
					if let Some(directory) = path.to_str() {
						directories.push(directory.to_string());
					}
					queue.push_back(path);
				}
			}
		}
	}

	directories
}
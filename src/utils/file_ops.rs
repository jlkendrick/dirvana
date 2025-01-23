use ignore::WalkBuilder;

/// Collects all directories in a given root directory
pub fn collect_directories(root: &str) -> Vec<String> {
    let mut directories = Vec::new();

    let walker = WalkBuilder::new(root)
        .hidden(true) // Skip dot directories by default
        .git_ignore(true) // Respect .gitignore files
        .build();

    // For each entry in the walker, check if it's a directory
    for result in walker {
        if let Ok(entry) = result {
            if entry.file_type().map(|ft| ft.is_dir()).unwrap_or(false) {
                // If the entry is a directory, get the relative path and add it to the list
                if let Ok(relative_path) = entry.path().strip_prefix(root) {
                    directories.push(relative_path.to_string_lossy().into_owned());
                }
            }
        }
    }

    directories
}
use ignore::WalkBuilder;

/// Collects all directories in a given root directory
pub fn collect_directories(root: &str) -> Vec<String> {
    let mut directories = Vec::new();

    let walker = WalkBuilder::new(root)
        .hidden(true) // Skip dot directories by default
        .git_ignore(true) // Respect .gitignore files
        .build();

    for result in walker {
        if let Ok(entry) = result {
            if entry.file_type().map(|ft| ft.is_dir()).unwrap_or(false) {
                directories.push(entry.path().to_string_lossy().into_owned());
            }
        }
    }

    directories
}
use crate::utils::paths::Paths;

use std::collections::HashMap;
use strsim::levenshtein;

// BKTree is a data structure that allows for efficient fuzzy searching
pub struct BKTree {
    tree: HashMap<String, Paths>,
}

impl BKTree {
    // Create a new BKTree
    pub fn new() -> Self {
        BKTree {
            tree: HashMap::new(),
        }
    }

    // Insert a path into the BKTree
    pub fn insert(&mut self, path: &str) {
        // Store the last directory in the path as the key so we can search by directory name
        let dir_name = path.split('/').last().unwrap().to_string();
        self.tree.entry(dir_name).or_insert(Paths::new()).add(path);
    }

    // Search for paths that are within a given distance of the query
    pub fn search(&self, query: &str, max_distance: usize) -> Vec<String> {
        // [TODO]: If more than the last directory was provided in the query, still search for the last directory
        // [TODO]: but then filter the results based on the remaining parent directories in the query

        // Search for keys that are within the given distance of the query
        self.tree
            .keys()
            .filter(|key| levenshtein(key, query) <= max_distance)
            .flat_map(|key| self.tree.get(key).unwrap().get_all())
            .collect()
    }

    // Access a path in the BKTree
    pub fn access(&mut self, path: &str) {
        let dir_name = path.split('/').last().unwrap().to_string();
        self.tree.entry(dir_name).or_insert(Paths::new()).access(path);
    }
}

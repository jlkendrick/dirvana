use std::collections::HashMap;
use strsim::levenshtein;

pub struct BKTree {
    tree: HashMap<String, String>,
}

impl BKTree {
    pub fn new() -> Self {
        BKTree {
            tree: HashMap::new(),
        }
    }

    pub fn insert(&mut self, path: &str) {
        let reversed_path = path.chars().rev().collect::<String>();
        self.tree.insert(reversed_path, path.to_string());
    }

    pub fn search(&self, query: &str, max_distance: usize) -> Vec<String> {
        let reversed_query = query.chars().rev().collect::<String>();
        self.tree
            .keys()
            .filter(|key| levenshtein(&reversed_query, key) <= max_distance)
            .filter_map(|key| self.tree.get(key)).cloned()
            .collect()
    }
}

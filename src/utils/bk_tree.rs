use crate::utils::paths::Paths;

use std::collections::HashMap;
use strsim::levenshtein;

pub struct BKTree {
    tree: HashMap<String, Paths>,
}

impl BKTree {
    pub fn new() -> Self {
        BKTree {
            tree: HashMap::new(),
        }
    }

    pub fn insert(&mut self, path: &str) {
        let dir_name = path.split('/').last().unwrap().to_string();
        self.tree.entry(dir_name).or_insert(Paths::new()).add(path);
    }

    pub fn search(&self, query: &str, max_distance: usize) -> Vec<String> {
        self.tree
            .keys()
            .filter(|key| levenshtein(key, query) <= max_distance)
            .flat_map(|key| self.tree.get(key).unwrap().get())
            .collect()
    }
}

use std::collections::HashMap;

#[derive(Default)]
pub struct InvertedIndex {
	index: HashMap<String, Vec<String>>
}

impl InvertedIndex {

	// Build an inverted index from a list of directories
	pub fn build(directories: &[String]) -> Self {
		let mut index = HashMap::new();

		for dir in directories {
			for token in dir.split('/') {
				let token = token.to_lowercase();
				let entry = index.entry(token).or_insert(Vec::new());
				entry.push(dir.clone());
			}
		}

		Self { index }
	}

	// Search the index for a given query
	pub fn search(&self, query: &str) -> Option<&Vec<String>> {
		self.index.get(&query.to_lowercase())
	}
}
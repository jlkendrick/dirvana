pub struct Paths {
		items: Vec<String>
}

impl Paths {
	pub fn new() -> Self {
		Paths {
			items: Vec::new()
		}
	}

	pub fn add(&mut self, path: &str) {
		self.items.push(path.to_string());
	}

	pub fn get(&self) -> Vec<String> {
		self.items.clone()
	}
}
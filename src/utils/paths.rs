// ------------------------------------- Paths -------------------------------------
// Paths is a data structure that stores a list of paths and prioritizes recently 
// accessed paths using a LRU cache
pub struct Paths {
		items: LRUCache
}

impl Paths {
	pub fn new() -> Self {
		Paths {
			items: LRUCache::new()
		}
	}

	// Add a path to the list of paths
	pub fn add(&mut self, path: &str) {
		self.items.add(path);
	}

	// Gets all paths in their current order determined by the LRU cache
	pub fn get_all(&self) -> Vec<String> {
		self.items.read_order()
	}

	// Indicates to the LRU cache that a path was accessed
	pub fn access(&mut self, path: &str) {
		self.items.access(path);
	}
}

// ------------------------------------- LRU Cache -------------------------------------
use std::collections::HashMap;
use std::cell::RefCell;
use std::rc::Rc;

type NodePtr = Rc<RefCell<Node>>;

// Node of a doubly linked list
#[derive(Clone)]
struct Node {
	value: String,
	prev: Option<NodePtr>,
	next: Option<NodePtr>,
}

impl Node {
	fn new(value: String) -> Self {
		Node {
			value,
			prev: None,
			next: None
		}
	}
}

// LRU cache
struct LRUCache {
	cache: HashMap<String, NodePtr>,
	head: Option<NodePtr>,
	tail: Option<NodePtr>
}

impl LRUCache {
	pub fn new() -> Self {
		LRUCache {
			cache: HashMap::new(),
			head: None,
			tail: None
		}
	}

	// Helper insert function to insert a node at the end of the list
	fn insert_back(&mut self, path: &str) {
		let new_node = Rc::new(RefCell::new(Node::new(path.to_string())));
		
		match self.tail.take() {
			Some(old_tail) => {
				new_node.borrow_mut().prev = Some(old_tail.clone());
				old_tail.borrow_mut().next = Some(new_node.clone());
				self.tail = old_tail.borrow().next.clone();
			}
			None => {
				self.head = Some(new_node.clone());
				self.tail = Some(new_node.clone());
			}
		}
	}

	// Helper insert function to insert a node at the front of the list
	fn insert_front(&mut self, path: &str) {
		let new_node = Rc::new(RefCell::new(Node::new(path.to_string())));

		match self.head.take() {
			Some(old_head) => {
				new_node.borrow_mut().next = Some(old_head.clone());
				old_head.borrow_mut().prev = Some(new_node.clone());
				self.head = Some(new_node.clone());
			}
			None => {
				self.head = Some(new_node.clone());
				self.tail = Some(new_node.clone());
			}
		}
	}

	// Helper function to remove a node from the list
	fn remove(&mut self, node: NodePtr) {
		let prev = node.borrow().prev.clone();
		let next = node.borrow().next.clone();

		if let Some(p) = prev.clone() {
			p.borrow_mut().next = next.clone();
		} else {
			self.head = next.clone();
		}

		if let Some(n) = next.clone() {
			n.borrow_mut().prev = prev;
		} else {
			self.tail = prev;
		}
	}

	// Get all paths in their current order determined by the LRU cache
	fn read_order(&self) -> Vec<String> {
		let mut paths = Vec::new();
		let mut current = self.head.clone();

		while let Some(node) = current {
			paths.push(node.borrow().value.clone());
			current = node.borrow().next.clone();
		}

		paths
	}

	// Add a path to the LRU cache. For use during initial insertion
	fn add(&mut self, path: &str) {
		if let Some(_) = self.cache.get(path) {
			return
		} else {
			self.insert_back(path);
			self.cache.insert(path.to_string(), self.tail.clone().unwrap());
		}
	}

	// Access a path, moving it to the front of the list
	fn access(&mut self, path: &str) {
		if let Some(node) = self.cache.get(path) {
			self.remove(node.clone());
			self.insert_front(path);
		}
	}

}
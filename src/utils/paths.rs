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
            next: None,
        }
    }
}

pub struct LRUCache {
    cache: HashMap<String, NodePtr>,
    head: Option<NodePtr>,
    tail: Option<NodePtr>,
}

impl LRUCache {
    pub fn new() -> Self {
        let head = Rc::new(RefCell::new(Node::new(String::new())));
        let tail = Rc::new(RefCell::new(Node::new(String::new())));

        head.borrow_mut().next = Some(tail.clone());
        tail.borrow_mut().prev = Some(head.clone());

        LRUCache {
            cache: HashMap::new(),
            head: Some(head),
            tail: Some(tail),
        }
    }

	// Inserts a node at the front of the LRU cache
    fn insert_front(&mut self, path: &str) {
        let new_node = Rc::new(RefCell::new(Node::new(path.to_string())));

        let old_head = self.head.as_ref().unwrap().borrow().next.clone();
        new_node.borrow_mut().next = old_head.clone();
        new_node.borrow_mut().prev = self.head.clone();
        self.head.as_ref().unwrap().borrow_mut().next = Some(new_node.clone());
        if let Some(old_head) = old_head {
            old_head.borrow_mut().prev = Some(new_node.clone());
        }
    }

	// Inserts a node at the back of the LRU cache
	fn insert_back(&mut self, path: &str) {
		let new_node = Rc::new(RefCell::new(Node::new(path.to_string())));
		
		let old_tail = self.tail.as_ref().unwrap().borrow().prev.clone();
		new_node.borrow_mut().prev = old_tail.clone();
		new_node.borrow_mut().next = self.tail.clone();
		self.tail.as_ref().unwrap().borrow_mut().prev = Some(new_node.clone());
		if let Some(old_tail) = old_tail {
			old_tail.borrow_mut().next = Some(new_node.clone());
		}
	}

	// Removes a node from the LRU cache
    fn remove(&mut self, node: NodePtr) {
        let prev = node.borrow().prev.clone();
        let next = node.borrow().next.clone();

        if let Some(p) = prev.clone() {
            p.borrow_mut().next = next.clone();
        }
        if let Some(n) = next {
            n.borrow_mut().prev = prev;
        }
    }

	// Indicates to the LRU cache that a path was accessed
    pub fn access(&mut self, path: &str) {
        if let Some(node) = self.cache.get(path) {
            self.remove(node.clone());
            self.insert_front(path);
            self.cache.insert(path.to_string(), self.head.as_ref().unwrap().borrow().next.clone().unwrap());
        }
    }

	// Adds a path to the LRU cache. Primarily used when constructing the BKTree  (during initialization)
    pub fn add(&mut self, path: &str) {
        if let Some(_) = self.cache.get(path) {
            self.access(path);
        } else {
            self.insert_back(path);
            self.cache.insert(path.to_string(), self.tail.as_ref().unwrap().borrow().prev.clone().unwrap());
        }
    }

	// Gets all paths in their current order determined by the LRU cache
    pub fn read_order(&self) -> Vec<String> {
        let mut paths = Vec::new();
        let mut current = self.head.as_ref().unwrap().borrow().next.clone();

        while let Some(node) = current {
            if !node.borrow().value.is_empty() {
                paths.push(node.borrow().value.clone());
            }
            current = node.borrow().next.clone();
        }

        paths
    }
}
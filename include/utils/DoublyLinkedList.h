#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include "Node.h"

#include <memory>
#include <string>

// DoublyLinkedList is a simple doubly linked list implementation that is used by the RecentlyAccessedCache
// to keep track of the order of the nodes in the cache.
class DoublyLinkedList {
private:

	std::shared_ptr<Node> head;
	std::shared_ptr<Node> tail;

public:

	// Constructs an empty DoublyLinkedList with dummy nodes for head and tail to
	// avoid edge cases when inserting or deleting nodes.
	DoublyLinkedList();
	~DoublyLinkedList() = default;

	// Inserts a node before the given node
	void insert_before(std::shared_ptr<Node> before, std::shared_ptr<Node> after);

	// Inserts a node at the front of the list
	void insert_front(std::shared_ptr<Node> node);

	// Inserts a node at the back of the list
	void insert_back(std::shared_ptr<Node> node);

	// Deletes the given node from the list using the node's pointers
	void delete_at(std::shared_ptr<Node> node);

	// Returns a vector of paths from all nodes in the list in order
	std::vector<std::string> get_all_paths() const;

	// Returns the dummy head of the list
	std::shared_ptr<Node> get_dummy_head() const { return head; }

	// Returns the dummy tail of the list
	std::shared_ptr<Node> get_dummy_tail() const { return tail; };
};

#endif // DOUBLYLINKEDLIST_H
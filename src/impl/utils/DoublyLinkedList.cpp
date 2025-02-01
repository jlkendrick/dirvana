#include "DoublyLinkedList.h"

#include "Node.h"

#include <memory>
#include <iostream>

DoublyLinkedList::DoublyLinkedList() {
	// Use dummy nodes to avoid edge cases
	head = std::make_shared<Node>("");
	tail = std::make_shared<Node>("");

	head->next = tail;
	tail->prev = head;
}

void DoublyLinkedList::insert_front(std::shared_ptr<Node> node) {
	// Assign outgoing pointers
	node->next = head->next;
	node->prev = head;

	// Assign incoming pointers
	head->next->prev = node;
	head->next = node;
}

void DoublyLinkedList::insert_back(std::shared_ptr<Node> node) {
	// Assign outgoing pointers
	node->next = tail;
	node->prev = tail->prev;

	// Assign incoming pointers
	tail->prev->next = node;
	tail->prev = node;
}

void DoublyLinkedList::insert_before(std::shared_ptr<Node> before, std::shared_ptr<Node> after) {
	// Assign outgoing pointers
	before->next = after;
	before->prev = after->prev;

	// Assign incoming pointers
	after->prev->next = before;
	after->prev = before;
}

void DoublyLinkedList::delete_at(std::shared_ptr<Node> node) {
	// Make sure the node is not a dummy node
	if (node == head || node == tail)
		return;
	
	// Move incoming pointers
	node->prev->next = node->next;
	node->next->prev = node->prev;
}

std::vector<std::string> DoublyLinkedList::get_all_paths() const {
	std::vector<std::string> paths;
	
	// Iterate through the list and add the paths to the vector
	std::shared_ptr<Node> current = head->next;
	while (current != tail) {
		paths.push_back(current->path);
		current = current->next;
	}

	return paths;
}

void DoublyLinkedList::clear() {
	// Clear the list by setting the head and tail to point to each other
	head->next = tail;
	tail->prev = head;
}
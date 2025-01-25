#include "DoublyLinkedList.h"

#include "Node.h"

DoublyLinkedList::DoublyLinkedList() {

	// Use dummy nodes to avoid edge cases
	head = new Node("");
	tail = new Node("");

	head->next = tail;
	tail->prev = head;

	size = 0;
}

DoublyLinkedList::~DoublyLinkedList() {
	Node* current = head;
	while (current != nullptr) {
		Node* next = current->next;
		delete current;
		current = next;
	}
}

void DoublyLinkedList::insert_front(Node* node) {
	node->next = head->next;
	node->prev = head;

	head->next->prev = node;
	head->next = node;

	size++;
}

void DoublyLinkedList::insert_back(Node* node) {
	node->next = tail;
	node->prev = tail->prev;

	tail->prev->next = node;
	tail->prev = node;

	size++;
}

void DoublyLinkedList::delete_at(Node* node) {
	// Make sure the node is not a dummy node
	if (node == head || node == tail)
		return;
	
	node->prev->next = node->next;
	node->next->prev = node->prev;

	delete node;
	size--;
}
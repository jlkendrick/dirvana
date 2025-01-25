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
	node->next = head->next;
	node->prev = head;

	head->next->prev = node;
	head->next = node;
}

void DoublyLinkedList::insert_back(std::shared_ptr<Node> node) {
	node->next = tail;
	node->prev = tail->prev;


	tail->prev->next = node;
	tail->prev = node;
}

void DoublyLinkedList::delete_at(std::shared_ptr<Node> node) {
	// Make sure the node is not a dummy node
	if (node == head || node == tail)
		return;
	
	node->prev->next = node->next;
	node->next->prev = node->prev;
}
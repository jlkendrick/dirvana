#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include "Node.h"

#include <memory>
#include <string>

using std::string;

class DoublyLinkedList {
public:

	// Constructor and destructor
	DoublyLinkedList();
	~DoublyLinkedList() = default;

	// Insertion
	void insert_front(std::shared_ptr<Node> node);
	void insert_back(std::shared_ptr<Node> node);

	// Deletion
	void delete_at(std::shared_ptr<Node> node);

private:
	std::shared_ptr<Node> head;
	std::shared_ptr<Node> tail;
	
};

#endif // DOUBLYLINKEDLIST_H
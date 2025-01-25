#ifndef DOUBLYLINKEDLIST_H
#define DOUBLYLINKEDLIST_H

#include "Node.h"

#include <string>

using std::string;

class DoublyLinkedList {
public:

	// Constructor and destructor
	DoublyLinkedList();
	~DoublyLinkedList();

	// Getters
	int get_size() const { return size; }

	// Insertion
	void insert_front(Node* node);
	void insert_back(Node* node);

	// Deletion
	void delete_at(Node* node);

private:
	Node* head;
	Node* tail;
	int size;
	
};

#endif // DOUBLYLINKEDLIST_H
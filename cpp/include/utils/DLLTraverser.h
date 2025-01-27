#ifndef DLLTRAVERSER_H
#define DLLTRAVERSER_H

#include "DoublyLinkedList.h"
#include "Node.h"

#include <memory>
#include <string>

class DLLTraverser {
public:
	// Constructs the traverser with a reference to the list
	DLLTraverser(const DoublyLinkedList& list);

	// Resets the cursor to the head of the list
	void reset_to_front();

	// Resets the cursor to the tail of the list
	void reset_to_back();

	// Returns the path at the current cursor position
	std::string current() const;

	// Moves the cursor to the next or previous node, resetting to the other end if necessary
	void move_next();
	void move_prev();


private:
	const DoublyLinkedList& list;
	std::shared_ptr<Node> cursor;
	
	// Checks if there is a next or previous node
	bool has_next() const;
	bool has_prev() const;
};

#endif
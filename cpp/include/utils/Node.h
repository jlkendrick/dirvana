#ifndef NODE_H
#define NODE_H

#include <memory>
#include <string>

// Node is a simple struct that holds a path and pointers to the next and previous nodes in a DoublyLinkedList
struct Node {
	std::string path;
	std::shared_ptr<Node> next;
	std::shared_ptr<Node> prev;

	Node(std::string path) : path(path), next(nullptr), prev(nullptr) {}
};

#endif // NODE_H
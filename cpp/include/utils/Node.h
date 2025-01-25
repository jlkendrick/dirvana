#ifndef NODE_H
#define NODE_H

#include <string>

struct Node {
	std::string path;
	Node* next;
	Node* prev;

	Node(std::string path) : path(path), next(nullptr), prev(nullptr) {}
};

#endif // NODE_H
#ifndef NODE_H
#define NODE_H

#include <memory>
#include <string>

struct Node {
	std::string path;
	std::shared_ptr<Node> next;
	std::shared_ptr<Node> prev;

	Node(std::string path) : path(path), next(nullptr), prev(nullptr) {}
};

#endif // NODE_H
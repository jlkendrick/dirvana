#include "DLLTraverser.h"

DLLTraverser::DLLTraverser(const DoublyLinkedList& list) : list(list), cursor(list.get_dummy_head()->next) {}

void DLLTraverser::reset_to_front() {
	cursor = list.get_dummy_head()->next;
}

void DLLTraverser::reset_to_back() {
	cursor = list.get_dummy_tail()->prev;
}

std::string DLLTraverser::current() const {
	return cursor->path;
}

void DLLTraverser::move_next() {
	// If we are at a dummy node, that means the list is empty so we can't move
	if (cursor == list.get_dummy_head() || cursor == list.get_dummy_tail())
		return;

	// If we can move to the next node, do so
	if (has_next())
		cursor = cursor->next;
	// Otherwise, reset to the head so we can cycle back to the front
	else
		reset_to_front();	
}

void DLLTraverser::move_prev() {
	// If we are at a dummy node, that means the list is empty so we can't move
	if (cursor == list.get_dummy_head() || cursor == list.get_dummy_tail())
		return;

	// If we can move to the previous node, do so
	if (has_prev())
		cursor = cursor->prev;
	// Otherwise, reset to the tail so we can cycle back to the back
	else
		reset_to_back();
}

bool DLLTraverser::has_next() const {
	return cursor != list.get_dummy_tail() && cursor->next != list.get_dummy_tail();
}

bool DLLTraverser::has_prev() const {
	return cursor!= list.get_dummy_head() && cursor->prev!= list.get_dummy_head();
}
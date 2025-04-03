#include "FrequencyBasedCache.h"

void FrequencyBasedCache::promote(const std::string& path) {
	auto it = cache[path];
	FBCEntry entry = *it;
		
	// Increment the access count
	entry.access_count++;
		
	// Remove the entry from its current position
	order.erase(it);
		
	// Find the position to insert based on access count (higher counts first)
	auto insert_pos = order.begin();
	while (insert_pos != order.end() && 
					insert_pos->access_count > entry.access_count) {
		++insert_pos;
	}
		
	// Insert the entry at the appropriate position
	auto new_it = order.insert(insert_pos, entry);
	cache[path] = new_it;
}
#include "Types.h"

ordered_json RACEntry::serialize() const {
	ordered_json j;
	j["path"] = path;
	return j;
}

ordered_json FBCEntry::serialize() const {
	ordered_json j;
	j["path"] = path;
	j["access_count"] = access_count;
	return j;
}
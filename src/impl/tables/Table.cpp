#include "tables/Table.h"
#include "Database.h"
#include "utils/Helpers.h"

std::string Table::get_query_pattern(const std::string& dir_name, const std::string& matching_type_override) const {
	switch (matching_type_override.empty() ? db.get_config().get_matching_type() : TypeConversions::s_to_matching_type(matching_type_override)) {
		case MatchingType::Exact:
			return "'" + dir_name + "'";
		case MatchingType::Prefix:
			return "'" + dir_name + "%'";
		case MatchingType::Suffix:
			return "'%" + dir_name + "'";
		case MatchingType::Contains:
			return "'%" + dir_name + "%'";
	}
	return "";
}


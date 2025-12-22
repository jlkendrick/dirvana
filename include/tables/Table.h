#ifndef TABLE_H
#define TABLE_H

#include <sqlite_modern_cpp.h>

class Database;  // Forward declaration


class Table {
public:
	Table(Database& db) : db(db) {}

	virtual void create_table() const = 0;
	virtual void drop_table() const = 0;
	virtual std::vector<std::string> query(const std::string& input) const = 0;
	virtual void access(const std::string& input) = 0;
	
	Database& db;

protected:
	std::string get_query_pattern(const std::string& dir_name, const std::string& matching_type_override = "") const;
};

#endif // TABLE_H
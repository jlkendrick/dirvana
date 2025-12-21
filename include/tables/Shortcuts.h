#ifndef SHORTCUTS_TABLE_H
#define SHORTCUTS_TABLE_H

#include "Table.h"

class ShortcutsTable : public Table {
public:
		ShortcutsTable(Database& db) : Table(db) {}

		void create_table() const override;
		void drop_table() const override;
		std::vector<std::string> query(const std::string& input) const override;
};

#endif // SHORTCUTS_TABLE_H
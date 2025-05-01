#ifndef DATABASE_H
#define DATABASE_H

#include "Config.h"

#include <sqlite_modern_cpp.h>


class Database {
public:
	Database(const Config& config);

private:
	sqlite::database db;
};

#endif // DATABASE_H
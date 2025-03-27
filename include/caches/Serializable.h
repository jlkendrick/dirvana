#ifndef SERIALIZABLE_H
#define SERIALIZABLE_H

#include <string>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class ISerializable {
public:
	virtual ~ISerializable() = default;
	virtual json serialize() const = 0;
};

#endif // SERIALIZABLE_H
#ifndef ISERIALIZABLE_H
#define ISERIALIZABLE_H

#include <string>
#include "nlohmann/json.hpp"

using ordered_json = nlohmann::ordered_json;

class ISerializable {
public:
	virtual ~ISerializable() = default;
	virtual ordered_json serialize() const = 0;
};

#endif // ISERIALIZABLE_H
#ifndef IPROMOTIONSTRATEGY_H
#define IPROMOTIONSTRATEGY_H

#include <string>


class IPromotionStrategy {
public:
	virtual void promote(const std::string& path) = 0;
};

#endif // IPROMOTIONSTRATEGY_H
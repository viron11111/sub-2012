#ifndef VALIDATION_GATE_FINDER_H
#define VALIDATION_GATE_FINDER_H

#include "IFinder.h"

class ValidationGateFinder : public IFinder
{
	public:
		ValidationGateFinder(vector<int> objectIDs, boost::shared_ptr<INormalizer> n, boost::shared_ptr<IThresholder> t);
		vector<boost::property_tree::ptree> find(IOImages* ioimages);
};

#endif

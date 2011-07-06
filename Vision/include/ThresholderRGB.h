#ifndef ITHRESHOLDER_RGB_ORANGE_H
#define ITHRESHOLDER_RGB_ORANGE_H

#include "IThresholder.h"

class ThresholderRGB : public IThresholder
{
public:
	ThresholderRGB(void);
	~ThresholderRGB(void);
	void thresh(IOImages* ioimages, int objectID);

private:
	void threshOrange(IOImages* ioimages);
	void threshGreen(IOImages* ioimages);
	void threshYellow(IOImages* ioimages);
	void threshBlack(IOImages* ioimages);
};

#endif
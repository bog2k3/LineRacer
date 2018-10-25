#ifndef __PAINTER_H__
#define __PAINTER_H__

#include "transform.h"

struct Color;

class Painter {
public:
	static void loadResources();

	// draws an arrow from point A to B with a head of size [headSize] pixels and [headAperture] aperture
	static void paintArrow(ScreenPoint const& from, ScreenPoint const& to, int headSize, float headAperture, Color const& c);
};

#endif

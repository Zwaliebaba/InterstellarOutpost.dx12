#ifndef _included_debug_render_h
#define _included_debug_render_h

#ifdef DEBUG_RENDER_ENABLED

#include "rgb_colour.h"
#include "vector3.h"

void RenderSphere(Vector3 const &_centre, float _radius, RGBAColour const &_col=RGBAColour(255,255,255));

void RenderVerticalCylinder(Vector3 const &_centreBase, Vector3 const &_verticalAxis,
							float _height, float _radius, 
							RGBAColour const &_col=RGBAColour(255,255,255));

void RenderArrow(Vector3 const &start, Vector3 const &end, float width, RGBAColour const &_col=RGBAColour(255,255,255));

#endif // DEBUG_RENDER_ENABLED

#endif

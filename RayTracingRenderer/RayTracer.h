#pragma once

#include <limits>

#include "Window.h"
#include "Vertex.h"
#include "SimpIO.h"
#include "Lighting.h"


struct polyIndex {
	int index;
	double t;
	vec4 position;
	vec4 lambdaVec;		// Contains the scalar information for point in convex hull
	bool isSet;

	polyIndex()
	{
		index = -1;
		t = std::numeric_limits<double>::max();
		position = vec4(0.0, 0.0, 0.0, 1.0);
		lambdaVec = vec4(0.0, 0.0, 0.0, 0.0);
		isSet = false;
	}

	polyIndex(int Index, double scalar, vec4 pos, vec4 lambdaVector)
	{
		index = Index;
		t = scalar;
		position = pos;
		lambdaVec = lambdaVector;
		isSet = true;
	}
};


class RayTracer
{
private:
	Window* _window;
	PolygonList* _polygons;
	FrustumParams _viewingPlane;
	int _maxRecursionDepth;
	double _scale;
	double _maxDist;
	Lighting* _lightEngine;

	vec4 getRay(int x, int y);

public:
	RayTracer(Window* window, Lighting* lightEngine, PolygonList* polygons, FrustumParams viewPlane);
	~RayTracer();

	void run();
	Color_f Trace(vec4 origin, vec4 ray, double weight, int currDepth);
};


#include "RayTracer.h"


/* RayTracer helper functions ------------------------------------------------------ */


Color_f calculateDepthShade(double Z, Color_f objColor)
{
	if (Z <= 100) { return objColor; }
	if (Z >= 1100) { return Color_f(0.0, 0.0, 0.0); }

	double ratio = (Z - 100) / 1000;

	double r = ((1.0 - ratio) * objColor.r);
	double g = ((1.0 - ratio) * objColor.g);
	double b = ((1.0 - ratio) * objColor.b);

	return Color_f(r, g, b);
}


bool isCCW(const std::vector<Vertex>& vertices)
{
	double result = 0.0;
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		vec4 v1 = vertices[i].pos_WS;
		vec4 v2 = vertices[(i + 1) % vertices.size()].pos_WS;
		result += (v1.x * v2.y) - (v2.x * v1.y);
	}
	return result < 0;
}


/* End helper functions ------------------------------------------------------------ */


RayTracer::RayTracer(Window* window, Lighting* lightEngine, PolygonList* polygons, FrustumParams viewPlane)
{
	_window = window;
	_lightEngine = lightEngine;
	_polygons = polygons;
	_viewingPlane = viewPlane;
	_maxRecursionDepth = 2;
	_scale = 0.5;
	_maxDist = 300;
	
	_lightEngine->setPolygonPtr(polygons);
}


RayTracer::~RayTracer()
{
	// Empty;
}


vec4 RayTracer::getRay(int x, int y)
{
	vec4 ray = vec4(0.0, 0.0, 0.0, 0.0);

	ray.x = _viewingPlane.xLo + x * (_viewingPlane.xHi - _viewingPlane.xLo) / _window->getWidth();
	ray.y = _viewingPlane.yHi - y * (_viewingPlane.yHi - _viewingPlane.yLo) / _window->getHeight();
	ray.z = 1.0;
	ray.w = 1.0;

	return ray;
}


Color_f RayTracer::Trace(vec4 origin, vec4 ray, double weight, int currDepth)
{
	if (currDepth >= _maxRecursionDepth)
	{
		return Color_f(0.0, 0.0, 0.0); //Should be ambient colour
	}

	polyIndex intersectingPoly;

	for (unsigned int i = 0; i < _polygons->polygons.size(); i++)
	{
		// Find t parameter of intersection
		vec4 plane = _polygons->polygons[i].plane;
		vec4 plane_D0 = plane;
		plane_D0.w = 0.0;

		double t = 0.0f;
		double denominator = plane_D0.dot(ray - origin);

		// Plane is not parallel with ray if denominator != 0
		if (denominator != 0)
		{
			t = -origin.dot(plane) / denominator;
			vec4 soln = origin + (ray - origin) * t;
			vec4 lambdaVec;
			if (inIntersection(_polygons->polygons[i].vertexData[0].pos_WS,
				_polygons->polygons[i].vertexData[1].pos_WS,
				_polygons->polygons[i].vertexData[2].pos_WS, soln, &lambdaVec) && t < intersectingPoly.t && t > 0.01)
																				// OR Some appropriate epsilon  ^^^
			{
				if (currDepth == 0 && !isCCW(_polygons->polygons[i].vertexData))
				{
					continue;
				}
				else
				{
					intersectingPoly = polyIndex(i, t, soln, lambdaVec);
				}
			}
		}
	}

	// Compute next ray

	if (intersectingPoly.isSet)
	{
		vec4 N = (_polygons->polygons[intersectingPoly.index].vertexData[0].normal * intersectingPoly.lambdaVec.x) +
				 (_polygons->polygons[intersectingPoly.index].vertexData[1].normal * intersectingPoly.lambdaVec.y) +
				 (_polygons->polygons[intersectingPoly.index].vertexData[2].normal * intersectingPoly.lambdaVec.z);
		N.normalize();

		vec4 V = (ray - origin) * -1.0;
		V.normalize();
		vec4 R = (N * 2.0f * N.dot(V)) - V;
		R.normalize();
		intersectingPoly.position.w = 1.0;

		Color C = _polygons->polygons[intersectingPoly.index].color;
		Color_f pointColor = Color_f(C.r, C.g, C.b);
		pointColor = _lightEngine->PerformLightingCalculation(pointColor, N, intersectingPoly.position);
		pointColor = calculateDepthShade(intersectingPoly.position.z, pointColor);

		if (currDepth == 0)
		{
			return pointColor + Trace(intersectingPoly.position, intersectingPoly.position + R, 1.0, ++currDepth);
		}
		else
		{
			weight = weight * (_maxDist - intersectingPoly.t) / _maxDist;
			return pointColor * _scale * weight + Trace(intersectingPoly.position, intersectingPoly.position + R, weight, ++currDepth);
		}
	}
	else
	{
		return Color_f(0.0, 0.0, 0.0); //Should be ambient colour
	}
}

void RayTracer::run()
{
	// Ray is from v to w_i. v is fixed at eyepoint.
	vec4 v = vec4(0.0, 0.0, 0.0, 1.0);

	// for (each pixel in window)
	for (int y = 0; y < _window->getHeight(); y++)
	{
		for (int x = 0; x < _window->getWidth(); x++)
		{
			// w_i is the ray
			vec4 w_i = getRay(x, y);

			Color_f color = Trace(v, w_i, 1.0, 0);
			if (color.r > 1.0) { color.r = 1.0; }
			if (color.g > 1.0) { color.g = 1.0; }
			if (color.b > 1.0) { color.b = 1.0; }
			Color pixelColor = color;

			_window->setPixel(x, y, pixelColor);

		}
		//if (y % 25 == 0) { std::cout << y << std::endl; }
	}
	std::cout << "Done." << std::endl;
	_window->RenderFrame();
}





/*Color RayTracer::Trace(vec4 origin, vec4 ray, double weight, int currDepth)
{
	//std::cout << currDepth << std::endl;

	if (currDepth >= _maxRecursionDepth)
	{
		return Color(0, 0, 0); //Should be ambient colour
	}

	polyIndex intersectingPoly;

	for (int i = 0; i < _polygons->polygons.size(); i++)
	{
		// Find t parameter of intersection
		vec4 plane = _polygons->polygons[i].plane;
		vec4 plane_D0 = plane;
		plane_D0.w = 0.0;

		double t = 0.0f;
		double denominator = plane_D0.dot(ray - origin);

		// Plane is not parallel with ray if denominator != 0
		if (denominator != 0)
		{
			t = -origin.dot(plane) / denominator;
			vec4 soln = origin + (ray - origin) * t;
			vec4 lambdaVec;
			if (inIntersection(_polygons->polygons[i].vertexData[0].pos_WS,
				_polygons->polygons[i].vertexData[1].pos_WS,
				_polygons->polygons[i].vertexData[2].pos_WS, soln, &lambdaVec) && t < intersectingPoly.t && t > 0.01)
																				// OR Some appropriate epsilon  ^^^
			{
				intersectingPoly = polyIndex(i, t, soln, lambdaVec);
			}
		}
	}

	// Compute next ray

	if (intersectingPoly.isSet)
	{
		vec4 N = (_polygons->polygons[intersectingPoly.index].vertexData[0].normal * intersectingPoly.lambdaVec.x) +
				 (_polygons->polygons[intersectingPoly.index].vertexData[1].normal * intersectingPoly.lambdaVec.y) +
				 (_polygons->polygons[intersectingPoly.index].vertexData[2].normal * intersectingPoly.lambdaVec.z);
		//vec4 N = _polygons->polygons[intersectingPoly.index].vertexData[0].normal;
		N.normalize();

		vec4 V = (ray - origin) * -1.0;
		//ray.normalize();
		V.normalize();
		//std::cout << ray.dot(V) << std::endl;

		//V.print();

		vec4 R = (N * 2.0f * (N.dot(V))) - V;

		R.normalize();

		//R.w = 1.0;
		intersectingPoly.position.w = 1.0;

		//N.print();
		//std::cout << intersectingPoly.index << std::endl << std::endl;
		
		//std::cout << V.dot(R) << std::endl;
		//V.print();
		//N.print();
		//R.print();
		//std::cout << std::endl;
		
		//std::cout << currDepth << std::endl;
		//Color additionalColor = Trace(intersectingPoly.position, intersectingPoly.position + R, 1.0, currDepth) * (0.25)*((90.0 - intersectingPoly.t) / 90.0);
		//(1 - N.dot(V)) * (10 / (currDepth * intersectingPoly.t));
		//std::cout << intersectingPoly.t << std::endl;
if (false)
{
	std::cout << currDepth << std::endl;
	//std::cout << (int)additionalColor.r << " " << (int)additionalColor.g << " " << (int)additionalColor.b << " " << std::endl;
	std::cout << std::endl;
}
//std::cout << (int)additionalColor.r << " " << (int)additionalColor.g << " " << (int)additionalColor.b << " " << std::endl;

Color pointColor = _polygons->polygons[intersectingPoly.index].color;
pointColor = _lightEngine->PerformLightingCalculation(pointColor, N, intersectingPoly.position);

if (currDepth == 0)
{
	return pointColor + Trace(intersectingPoly.position, intersectingPoly.position + R, 1.0, ++currDepth);
}
else
{
	//return Color(0, 0, 0);
	weight = weight * (_maxDist - intersectingPoly.t) / _maxDist;
	//std::cout << weight << std::endl;
	return pointColor * 0.1 * weight + Trace(intersectingPoly.position, intersectingPoly.position + R, weight, ++currDepth);

}
	}
	else
	{
		return Color(0, 0, 0); //Should be ambient colour
	}
}*/






/*
void RayTracer::run()
{
	// Ray from v to w_i. v is fixed.
	vec4 v = vec4(0.0, 0.0, 0.0, 1.0);

	for (int y = 0; y < _window->getHeight(); y++)
	{
		for (int x = 0; x < _window->getWidth(); x++)
		{
			// w_i is the ray
			vec4 w_i = getRay(x, y);
			polyIndex intersectingPoly;

			for (int i = 0; i < _polygons->polygons.size(); i++)
			{
				// Find t parameter of intersection
				vec4 plane = _polygons->polygons[i].plane;
				vec4 plane_D0 = plane;
				plane_D0.w = 0.0;

				double t = 0.0f;
				double denominator = plane_D0.dot(w_i - v);

				// Plane is not parallel with ray if denominator != 0
				if (denominator != 0)
				{
					t = -v.dot(plane) / denominator;
					vec4 soln = v + (w_i - v) * t;
					if (inIntersection(_polygons->polygons[i].vertexData[0].pos_WS,
									   _polygons->polygons[i].vertexData[1].pos_WS,
							           _polygons->polygons[i].vertexData[2].pos_WS, soln) && t < intersectingPoly.t)
					{
						intersectingPoly = polyIndex(i, t);
					}
				}
			}

			if (intersectingPoly.isSet)
			{
				_window->setPixel(x, y, _polygons->polygons[intersectingPoly.index].color);
			}
		}
	}
	_window->RenderFrame();
}
*/





/*
void RayTracer::run()
{
	// Ray from v to w_i. v is fixed.
	vec4 v = vec4(0.0, 0.0, 0.0, 1.0);

	vec4 c1 = vec4(1, 2, 3, 0);
	vec4 c2 = vec4(2, 6, 3, 0);
	vec4 c3 = vec4(7, 2, 1, 0);
	vec4 sol = vec4(2, 4, 1, 0);

	//ConvexCombination(c1, c2, c3, sol);

	for (int y = 0; y < _window->getHeight(); y++) //_window->getHeight()
	{
		for (int x = 0; x < _window->getWidth(); x++)
		{
			// w_i is the ray
			vec4 w_i = getRay(x, y);
			//v.print();
			//w_i.print();
			//vec4 diff = w_i - v;
			//diff.print();
			//std::cout << std::endl;

			double t = std::numeric_limits<double>::max();
			int polyIndex = -1;

			for (int i = 0; i < _polygons->polygons.size(); i++)
			{
				// Find t parameter of intersection
				vec4 plane = _polygons->polygons[i].plane;
				//plane.print();
				vec4 plane_D0 = plane;
				plane_D0.w = 0.0;

				double denominator = plane_D0.dot(w_i - v);

				// Plane is not parallel with ray if denominator != 0
				if (denominator != 0)
				{
					double new_t = -v.dot(plane) / denominator;
					if (new_t > 1.0 && new_t <= t)
					{
						t = new_t;
						polyIndex = i;
						//std::cout << t << std::endl;
					}
					//std::cout << t << std::endl;
				}
				//std::cout << std::endl;
			}

			vec4 soln = v + (w_i - v) * t;
			//soln.print();
			if (polyIndex >= 0 && inIntersection(_polygons->polygons[polyIndex].vertexData[0].pos_WS,
				_polygons->polygons[polyIndex].vertexData[1].pos_WS,
				_polygons->polygons[polyIndex].vertexData[2].pos_WS, soln))
			{
				//std::cout << new_t << std::endl;
				_window->setPixel(x, y, _polygons->polygons[polyIndex].color);
				//std::cout << t << std::endl;
				//system("Pause");
			}
		}
	}
	_window->RenderFrame();
}
*/
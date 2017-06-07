#include "Lighting.h"

#include "utility.h"


#include <iostream>


/* LightSource Methods ------------------------------------------------------------- */


LightSource::LightSource()
{
    position = vec4(0.0, 0.0, 0.0, 0.0);
    A = 0.0;
    B = 0.0;
    I = Color_f();
}


LightSource::LightSource(const vec4& pos, double A, double B, const Color_f& intensity)
{
    position = pos;
    position.hgDivision();
    position.w = 0.0;
    this->A = A;
    this->B = B;
    I = intensity;

    diffuse = 1.0;
    specular = 1.0;
    f_att = 1.0;
}


LightSource::~LightSource()
{
    //Empty
}


inline bool Lighting::inShadow(vec4 vertexPos, vec4 posPlusL)
{
	//vertexPos.print();
	//posPlusL.print();
	//std::cout << std::endl;

	for (unsigned int i = 0; i < _polygons->polygons.size(); i++)
	{
		// Find t parameter of intersection
		vec4 plane = _polygons->polygons[i].plane;
		vec4 plane_D0 = plane;
		plane_D0.w = 0.0;

		double t = 0.0f;
		double denominator = plane_D0.dot(posPlusL - vertexPos);

		// Plane is not parallel with ray if denominator != 0
		if (denominator != 0)
		{
			t = -vertexPos.dot(plane) / denominator;
			vec4 soln = vertexPos + (posPlusL - vertexPos) * t;
			vec4 lambdaVec;
			if (inIntersection(_polygons->polygons[i].vertexData[0].pos_WS,
				_polygons->polygons[i].vertexData[1].pos_WS,
				_polygons->polygons[i].vertexData[2].pos_WS, soln, &lambdaVec) && t > 0.01)
				// OR Some appropriate epsilon  ^^^
			{
				return true;
			}
		}
	}
	return false;
}

inline Color_f Lighting::ComputeVertexShadingColor(Color_f objectColor, vec4 vertexPos)
{
    Color_f result = objectColor;
    result = result * _ambient;

    for (unsigned int i = 0; i < _lightSources.size(); i++)
    {
        // Light does not hit face if 0
        if (_lightSources[i].diffuse <= 0) { continue; }

		vec4 L = _lightSources[i].position - vertexPos;
		L.w = 0.0;
		L.normalize();
		vertexPos.w = 1.0;
		if (inShadow(vertexPos, vertexPos + L)) { continue; }

        // R
        double I_r = _lightSources[i].I.r;
        double k_r = objectColor.r;
        double R = I_r * _lightSources[i].f_att * (k_r * _lightSources[i].diffuse + _lightSources[i].specular);
        // G
        double I_g = _lightSources[i].I.g;
        double k_g = objectColor.g;
        double G = I_g * _lightSources[i].f_att * (k_g * _lightSources[i].diffuse + _lightSources[i].specular);
        // B
        double I_b = _lightSources[i].I.b;
        double k_b = objectColor.b;
        double B = I_b * _lightSources[i].f_att * (k_b * _lightSources[i].diffuse + _lightSources[i].specular);

        result.r += R;
        result.g += G;
        result.b += B;
    }

    if (result.r > 1.0) {result.r = 1.0;}
    if (result.g > 1.0) {result.g = 1.0;}
    if (result.b > 1.0) {result.b = 1.0;}

	return result;
}


/* Lighting Methods ---------------------------------------------------------------- */


Lighting::Lighting()
{
    _model = LIGHTMODEL::FLAT;
    _k_s = 0.3f;
    _p = 8;
    _ambient = Color_f(0.0, 0.0, 0.0);
	doLighting = true;
	_polygons = nullptr;
}


Lighting::~Lighting()
{
    _lightSources.clear();
}


void Lighting::setSurfaceProperties(double K_s, int P)
{
    _k_s = K_s;
    _p = P;

	std::cout << "Light Surface Properties - K_s, P: " << K_s << " " << P << std::endl;
}


void Lighting::setAmbientLight(const Color_f& color)
{
    _ambient = color;
}


void Lighting::setLightModel(LIGHTMODEL model)
{
    _model = model;
}


void Lighting::setEyePoint(const vec4& eyePoint)
{
    _eyepoint = eyePoint;
    _eyepoint.hgDivision();
    _eyepoint.w = 0.0;
}

void Lighting::setPolygonPtr(PolygonList* polygons)
{
	_polygons = polygons;
}


void Lighting::addLightSource(const vec4& pos, double A, double B, const Color_f& intensity)
{
    _lightSources.push_back(LightSource(pos, A, B, intensity));
	std::cout << "Add Light Source - A, B: " << A << " " << B << std::endl;
}


Color_f Lighting::PerformLightingCalculation(Color_f objectColor, vec4& N, vec4 vertexPos)
{
	if (!doLighting) { return objectColor; }

    //Color_f object_fl = objectColor.convertTodouble();
    //Color_f result = object_fl;
    //result = result * _ambient;

    vec4 V;
    N.normalize();
    vertexPos.w = 0;

    V = _eyepoint - vertexPos;
    V.normalize();

    for (unsigned int i  = 0; i < _lightSources.size(); i++)
    {
        vec4 L = _lightSources[i].position - vertexPos;
        double dist = L.length();
        _lightSources[i].f_att =  1.0 / (_lightSources[i].A + _lightSources[i].B * dist);
        L.normalize();
        vec4 R;
        R = (N * 2.0f * (N.dot(L))) - L;
        R.normalize();

        _lightSources[i].diffuse = N.dot(L);

		// Clamp V.R if < 0
		double VdotR = V.dot(R);
		if (VdotR < 0.0) { VdotR = 0.0; }

        _lightSources[i].specular = _k_s * pow(V.dot(R), _p);
    }

	return ComputeVertexShadingColor(objectColor, vertexPos);

}






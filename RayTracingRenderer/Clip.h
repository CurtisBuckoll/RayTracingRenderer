/*
#pragma once

#include "Matrix.h"
#include "DynamicArray.h"
#include "Vertex.h"

#include <vector>

const int NUM_PLANES = 6;

class Clip {
private:
    vec4 _frustum[NUM_PLANES];

	inline bool cullPlane(const std::vector<Vertex>& vertices, const vec4& plane);
	inline void clipPlane(std::vector<Vertex>* vertices, const vec4& plane);

public:
	FrustumParams parameters;

    Clip();
    ~Clip();

	void Init(FrustumParams params);
    bool cullToFrustum(const std::vector<Vertex>& vertices);
    void clipToFrustum(std::vector<Vertex>* vertices);
	bool backFaceCull(const std::vector<Vertex>& vertices);
};
*/
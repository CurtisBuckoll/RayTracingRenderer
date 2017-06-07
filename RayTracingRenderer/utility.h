#pragma once

#include <iostream>
#include <random>

#include "zBuffer.h"
#include "Vertex.h"

// Probably put this somewhere better
struct FrustumParams {
	double hither;
	double yon;
	double xLo;
	double xHi;
	double yLo;
	double yHi;

	FrustumParams()
	{
		hither = yon = xLo = xHi = yLo = yHi = 0.0;
	}

	FrustumParams(double Hither, double Yon, double XLo, double YLo, double XHi, double YHi) :
		hither(Hither),
		yon(Yon),
		xLo(XLo),
		yLo(YLo),
		xHi(XHi),
		yHi(YHi)
	{
		// Empty
	}
};


/**	Computes cross product between the difference of the three vertices passed in:
 *	Performs (V2 - V1) x (V3 - V1)
 *	W component set to 0
 */
vec4 cross(const vec4& V1, const vec4& V2, const vec4& V3);

unsigned int getRandomColor();

int getRandomNumber(int min, int max);

unsigned int packColor(unsigned char r, unsigned char g, unsigned char b);
unsigned int packColor(Color color);

void unpackColor(unsigned int color, unsigned char* r, unsigned char* g, unsigned char* b);
Color unpackColor(unsigned int color);

// Multiply color values COMPONENT wise
Color multiplyColors(Color c1, Color c2);

// Multiply color values COMPONENT wise - with one doubleing point
Color multiplyColors(Color c1, Color_f c2);

inline void swapRows4x3(double mat3[4][3], int r1, int r2)
{
	double temp;
	for (int x = 0; x < 4; x++)
	{
		temp = mat3[x][r1];
		mat3[x][r1] = mat3[x][r2];
		mat3[x][r2] = temp;
	}
}


inline void print3x4(double mat[3][3])
{
	for (int y = 0; y < 3; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			std::cout << mat[x][y] << "\t\t";
		}
		std::cout << std::endl;
	}
}

inline vec4 ConvexCombination(const vec4& v1, const vec4& v2, const vec4& v3, vec4 soln)
{
	double aug_mat3x4[4][3];
	int cols = 3;
	int rows = 4;

	// Assign matrix entries
	aug_mat3x4[0][0] = v1.x; aug_mat3x4[1][0] = v2.x; aug_mat3x4[2][0] = v3.x; aug_mat3x4[3][0] = soln.x;
	aug_mat3x4[0][1] = v1.y; aug_mat3x4[1][1] = v2.y; aug_mat3x4[2][1] = v3.y; aug_mat3x4[3][1] = soln.y;
	aug_mat3x4[0][2] = v1.z; aug_mat3x4[1][2] = v2.z; aug_mat3x4[2][2] = v3.z; aug_mat3x4[3][2] = soln.z;

	//print3x4(aug_mat3x4);

	double c;
	for (int i = 0; i < 3; i++)
	{
		for (int y = i + 1; y < 3; y++)
		{
			if (aug_mat3x4[i][i] == 0)
			{
				// swap rows
				for (int k = i + 1; k < 3; k++)
				{
					if (aug_mat3x4[i][k] != 0)
					{
						swapRows4x3(aug_mat3x4, k, i);
					}
				}
			}

			c = aug_mat3x4[i][y] / aug_mat3x4[i][i];

			for (int x = i; x < 4; x++)
			{
				aug_mat3x4[x][y] = aug_mat3x4[x][y] - c * aug_mat3x4[x][i];
			}
		}
	}

	// back substitute
	for (int i = 2; i > 0; i--)
	{
		for (int y = i - 1; y >= 0; y--)
		{
			c = aug_mat3x4[i][y] / aug_mat3x4[i][i];

			for (int x = i; x < 4; x++)
			{
				aug_mat3x4[x][y] = aug_mat3x4[x][y] - c * aug_mat3x4[x][i];
			}
		}
	}

	// reduce diagonal to 1's
	for (int y = 0; y < 3; y++)
	{
		c = 1.0 / aug_mat3x4[y][y];
		aug_mat3x4[y][y] = c * aug_mat3x4[y][y];
		aug_mat3x4[3][y] = c * aug_mat3x4[3][y];
	}

	//print3x4(aug_mat3x4);

	return vec4(aug_mat3x4[3][0], aug_mat3x4[3][1], aug_mat3x4[3][2], 0.0);
}


inline bool inIntersection(const vec4& v1, const vec4& v2, const vec4& v3, vec4 soln, vec4* lambdaVec)
{
	vec4 linCombination = ConvexCombination(v1, v2, v3, soln);

	*lambdaVec = linCombination;

	// if true, the point lies in the convex hull of the polygon
	if (linCombination.x >= 0.0 && linCombination.x <= 1.0 &&
		linCombination.y >= 0.0 && linCombination.y <= 1.0 &&
		linCombination.z >= 0.0 && linCombination.z <= 1.0)
	{
		return true;
	}

	return false;
}
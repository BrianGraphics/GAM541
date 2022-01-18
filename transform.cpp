////////////////////////////////////////////////////////////////////////
// A small library of 4x4 matrix operations needed for graphics
// transformations.  glm::mat4 is a 4x4 float matrix class with indexing
// and printing methods.  A small list or procedures are supplied to
// create Rotate, Scale, Translate, and Perspective matrices and to
// return the product of any two such.

#include <glm/glm.hpp>

#include "math.h"
#include "transform.h"

float* Pntr(glm::mat4& M)
{
    return &(M[0][0]);
}

//@@ The following procedures should calculate and return 4x4
//transformation matrices instead of the identity.

// Return a rotation matrix around an axis (0:X, 1:Y, 2:Z) by an angle
// measured in degrees.  NOTE: Make sure to convert degrees to radians
// before using sin and cos.  HINT: radians = degrees*PI/180
const float pi = 3.14159f;
glm::mat4 Rotate(const int i, const float theta)
{
    glm::mat4 R;
	float radians = theta * pi / 180.0f;
	int j = (i + 1) % 3, k = (i + 2) % 3;
	// column based
	// 00 10 20 30
	// 01 11 21 31
	// 02 12 22 32
	// 03 13 23 33
	

	R[j][j] = cosf(radians);
	R[k][k] = cosf(radians);
	R[i][i] = 1;
	R[3][3] = 1;
	R[k][j] = (-1.0f) * sinf(radians);
	R[j][k] = sinf(radians);

	
    return R;
}

// Return a scale matrix
glm::mat4 Scale(const float x, const float y, const float z)
{
    glm::mat4 S;

	S[0][0] = x;
	S[1][1] = y;
	S[2][2] = z;
	S[3][3] = 1.0f;

    return S;
}

// Return a translation matrix
glm::mat4 Translate(const float x, const float y, const float z)
{
    glm::mat4 T;
	
	T[0][0] = 1.0f;
	T[1][1] = 1.0f;
	T[2][2] = 1.0f;
	T[3][3] = 1.0f;

	T[3][0] = x;
	T[3][1] = y;
	T[3][2] = z;

    return T;
}

// Returns a perspective projection matrix
glm::mat4 Perspective(const float rx, const float ry,
             const float front, const float back)
{
    glm::mat4 P;

	P[0][0] = 1.0f / rx;
	P[1][1] = 1.0f / ry;
	P[2][2] = (-1.0f) * (back + front) / (back - front);
	P[3][2] = (-1.0f) * (2 * back * front) / (back - front);
	P[2][3] = -1.0f;
	P[3][3] = 0.0f;

    return P;
}

glm::mat4 LookAt(const glm::vec3 Eye, const glm::vec3 Center, const glm::vec3 Up)
{
	glm::mat4 L;
	glm::vec3 V = glm::normalize(Center - Eye);
	glm::vec3 A = glm::normalize(glm::cross(V, glm::vec3(0.0,0.0,1.0)));
	glm::vec3 B = glm::cross(A, V);
	V *= -1.0;
	L = glm::mat4(A.x, B.x, V.x, 0.0,
				  A.y, B.y, V.y, 0.0,
				  A.z, B.z, V.z, 0.0,
				  0.0, 0.0, 0.0, 1.0);
	L *= Translate(Eye.x * -1.0, Eye.y * -1.0, Eye.z * -1.0);

	return L;
}



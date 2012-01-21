
#ifndef __VECTOR_HPP
#define __VECTOR_HPP

#include <gccore.h>
#include <math.h>

class Vector3D : public guVector
{
public:
	Vector3D(void)
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
	}

	Vector3D(const guVector &v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Vector3D(float px, float py, float pz)
	{
		x = px;
		y = py;
		z = pz;
	}

	Vector3D(float px, float py)
	{
		x = px;
		y = py;
		z = 0.f;
	}

	float sqNorm(void) const
	{
		return x * x + y * y + z * z;
	}

	float norm(void) const
	{
		return sqrt(sqNorm());
	}

	Vector3D operator-(const Vector3D &v) const
	{
		return Vector3D(x - v.x, y - v.y, z - v.z);
	}

	Vector3D operator+(const Vector3D &v) const
	{
		return Vector3D(x + v.x, y + v.y, z + v.z);
	}

	bool operator!=(const Vector3D &v) const
	{
		return fabs(x - v.x) > 0.f || fabs(y - v.y) > 0.f || fabs(z - v.z) > 0.f;
	}

	bool operator==(const Vector3D &v) const
	{
		return fabs(x - v.x) == 0.f && fabs(y - v.y) == 0.f && fabs(z - v.z) == 0.f;
	}

	Vector3D &operator-=(const Vector3D &v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3D &operator+=(const Vector3D &v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vector3D &operator*=(const Vector3D &v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vector3D operator/(float f) const
	{
		return f == 0.f ? *this : Vector3D(x / f, y / f, z / f);
	}

	Vector3D operator*(const Vector3D &v) const
	{
		return Vector3D(x * v.x, y * v.y, z * v.z);
	}

	Vector3D operator*(float f) const
	{
		return Vector3D(x * f, y * f, z * f);
	}

	Vector3D unit(void) const
	{
		return operator/(norm());
	}

	Vector3D operator-(void) const
	{
		return Vector3D(-x, -y, -z);
	}
	
	Vector3D rotateX(float angle) const
	{
		angle *= 0.01745329251994329577;
		float c = cos(angle);
		float s = sin(angle);
		return Vector3D(x, y * c - z * s, z * c + y * s);
	}

	Vector3D rotateY(float angle) const
	{
		angle *= 0.01745329251994329577;
		float c = cos(angle);
		float s = sin(angle);
		return Vector3D(x * c + z * s, y, z * c - x * s);
	}

	Vector3D rotateZ(float angle) const
	{
		angle *= 0.01745329251994329577;
		float c = cos(angle);
		float s = sin(angle);
		return Vector3D(x * c - y * s, y * c + x * s, z);
	}

	Vector3D rotateX(float c, float s) const
	{
		return Vector3D(x, y * c - z * s, z * c + y * s);
	}

	Vector3D rotateY(float c, float s) const
	{
		return Vector3D(x * c + z * s, y, z * c - x * s);
	}

	Vector3D rotateZ(float c, float s) const
	{
		return Vector3D(x * c - y * s, y * c + x * s, z);
	}

	float dot(const Vector3D &v) const
	{
		return x * v.x + y * v.y + z * v.z;
	}

	Vector3D cross(const Vector3D &v) const
	{
		return Vector3D(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

#endif // !defined(__VECTOR_HPP)

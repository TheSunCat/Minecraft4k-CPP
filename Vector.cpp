#include "Vector.h"
#include <SDL/SDL.h> // for abs

#include "Util.h"

// vec3

void vec3::zero()
{
    x = 0; y = 0; z = 0;
}

void vec3::normalize()
{
    float magnitude = sqrt(x*x + y*y + z*z);

    if(magnitude == 0)
        return;

    x /= magnitude;
    y /= magnitude;
    z /= magnitude;
}

vec3 vec3::normalized()
{
    vec3 ret = *this;
    ret.normalize();
    return ret;
}

vec3 vec3::normalize(const vec3& vec)
{
    vec3 ret = vec;
    ret.normalize();

    return ret;
}

vec3 vec3::sign()
{
    using ::sign;

    return vec3(
        sign(x),
        sign(y),
        sign(z)
    );
}


vec3 vec3::abs()
{
    using ::abs;

    return vec3(
        abs(x),
        abs(y),
        abs(z)
    );
}

vec3 vec3::fract()
{
    using ::fract;

    return vec3(
        fract(x),
        fract(y),
        fract(z)
    );
}

float vec3::dot(const vec3& left, const vec3& right)
{
    return left.x*right.x + left.y*right.y + left.z*right.z;
}

float vec3::dot(const vec3& other)
{
    return vec3::dot(*this, other);
}

vec3 vec3::cross(const vec3& left, const vec3& right)
{
    return vec3(
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x
    );
}

vec3 vec3::cross(const vec3& other)
{
    return vec3::cross(*this, other);
}

vec3 vec3::operator+(const vec3& other) const
{
    return vec3(x + other.x, y + other.y, z + other.z);
}

void vec3::operator+=(const vec3& other)
{
    *this = *this + other;
}

vec3 vec3::operator-(const vec3& other) const
{
    return vec3(x - other.x, y - other.y, z - other.z);
}

void vec3::operator-=(const vec3& other)
{
    *this = *this - other;
}

vec3 vec3::operator*(const vec3& other) const
{
    return vec3(x * other.x, y * other.y, z * other.z);
}

void vec3::operator*=(const vec3& other)
{
    *this = *this * other;
}

vec3 vec3::operator*(float factor) const
{
    return vec3(x * factor, y * factor, z * factor);
}

void vec3::operator*=(float factor)
{
    *this = *this * factor;
}

vec3 vec3::operator/(float factor) const
{
    return vec3(x / factor, y / factor, z / factor);
}

void vec3::operator/=(float factor)
{
    *this = *this / factor;
}

float vec3::operator[](int axis)
{
    switch(axis)
    {
    case 0:
        return x;
    case 1:
        return y;
    case 2:
        return z;
    default:
        return 0;
    }
}

vec3 operator/(float val, const vec3& vec)
{
    return vec3(
        val / vec.x,
        val / vec.y,
        val / vec.z
    );
}


// vec2

void vec2::zero()
{
    x = 0; y = 0;
}

void vec2::normalize()
{
    float magnitude = sqrt(x*x + y*y);

    if(magnitude == 0)
        return;

    x /= magnitude;
    y /= magnitude;
}

float vec2::dot(const vec2& left, const vec2& right)
{
    return left.x*right.x + left.y*right.y;
}

float vec2::dot(const vec2& other)
{
    return vec2::dot(*this, other);
}

vec2 vec2::operator+(const vec2& other) const
{
    return vec2(x + other.x, y + other.y);
}

void vec2::operator+=(const vec2& other)
{
    *this = *this + other;
}

vec2 vec2::operator-(const vec2& other) const
{
    return vec2(x - other.x, y - other.y);
}

void vec2::operator-=(const vec2& other)
{
    *this = *this - other;
}

vec2 vec2::operator*(float factor) const
{
    return vec2(x * factor, y * factor);
}

void vec2::operator*=(float factor)
{
    *this = *this * factor;
}

vec2 vec2::operator/(float factor) const
{
    return vec2(x / factor, y / factor);
}

void vec2::operator/=(float factor)
{
    *this = *this / factor;
}

vec2 vec2::operator/(const vec2& other) const
{
    return vec2(x / other.x, y / other.y);
}

void vec2::operator/=(const vec2& other)
{
    *this = *this / other;
}
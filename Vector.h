#pragma once

class vec3
{
public:
    float x = 0, y = 0, z = 0;

    constexpr vec3() = default;
    constexpr vec3(float val) : x(val), y(val), z(val) {}
    constexpr vec3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

    void zero();

    void normalize();
    vec3 normalized();
    static vec3 normalize(const vec3& vec);

    vec3 sign();
    vec3 abs();
    vec3 fract();

    static float dot(const vec3& left, const vec3& right);
    float dot(const vec3& other);

    static vec3 cross(const vec3& left, const vec3& right);
    vec3 cross(const vec3& other);

    // operators
    vec3 operator+(const vec3& other) const;
    void operator+=(const vec3& other);

    vec3 operator-(const vec3& other) const;
    void operator-=(const vec3& other);

    vec3 operator*(const vec3& other) const;
    void operator*=(const vec3& other);

    vec3 operator*(float factor) const;
    void operator*=(float factor);

    vec3 operator/(float factor) const;
    void operator/=(float factor);

    float operator[](int axis);
};

vec3 operator/(float val, const vec3& vec);

class vec2
{
public:
    float x = 0, y = 0;

    constexpr vec2() = default;
    constexpr vec2(float val) : x(val), y(val) {}
    constexpr vec2(float xVal, float yVal) : x(xVal), y(yVal) {}

    void zero();

    void normalize();

    static float dot(const vec2& left, const vec2& right);
    float dot(const vec2& other);

    // operators
    vec2 operator+(const vec2& other) const;
    inline void operator+=(const vec2& other);

    vec2 operator-(const vec2& other) const;
    inline void operator-=(const vec2& other);

    vec2 operator*(float factor) const;
    inline void operator*=(float factor);

    vec2 operator/(float factor) const;
    inline void operator/=(float factor);

    vec2 operator/(const vec2& other) const;
    inline void operator/=(const vec2& other);
};
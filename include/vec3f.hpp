#ifndef VEC3F_H
#define VEC3F_H

#include <cmath>

struct vec3f{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    vec3f() : x(0), y(0), z(0) { }
    vec3f(float x, float y, float z) : x(x), y(y), z(z) { }

    /* overloads */
    inline vec3f operator + (const vec3f &v) const { return vec3f(this->x + v.x,
                                                                        this->y + v.y,
                                                                        this->z + v.z); }
    inline vec3f operator - (const vec3f &v) const { return vec3f(this->x - v.x,
                                                                        this->y - v.y,
                                                                        this->z - v.z); }
    inline vec3f operator * (const vec3f &v) const { return vec3f(this->x * v.x,
                                                                        this->y * v.y,
                                                                        this->z * v.z); }
    inline vec3f operator * (const float &k) const { return vec3f(this->x * k, this->y * k, this->z * k); }
    inline vec3f operator / (const float &k) const { return vec3f(this->x / k, this->y / k, this->z / k); }
    inline vec3f &operator += (const vec3f &v){
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        return *this;
    }
    inline vec3f &operator -= (const vec3f &v){
        this->x -= v.x;
        this->y -= v.y;
        this->z -= v.z;
        return *this;
    }
    inline vec3f &operator *= (const float k){
        this->x *= k;
        this->y *= k;
        this->z *= k;
        return *this;
    }
    inline vec3f &operator /= (const float k){
        this->x /= k;
        this->y /= k;
        this->z /= k;
        return *this;
    }
    inline vec3f &operator = (const vec3f &v){
        if(this == &v){
            return *this;
        }

        this->x = v.x;
        this->y = v.y;
        this->z = v.z;

        return *this;
    }
    /* end overloads */
    inline float length() const { return sqrt(x * x + y * y + z * z); }

    inline vec3f norm() const {
        float r = 1 / length();
        return vec3f(x * r, y * r, z * r);
    }

    inline void normlize(){
        float r = 1 / length();
        this->x *= r;
        this->y *= r;
        this->z *= r;
    }
    /* dot product */
    inline float dot(const vec3f &v) const { return this->x * v.x + this->y * v.y + this->z *v.z; }
    /* cross product */
    inline vec3f cross(const vec3f &v) const { return vec3f(this->y * v.z - this->z * v.y,
                                                            this->z * v.x - this->x * v.z,
                                                            this->x * v.y - this->y * v.x); }
};

#endif

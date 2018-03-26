//
// Created by zsmyn on 05-May-16.
//

#ifndef VERMILION_TYPES_H
#define VERMILION_TYPES_H

namespace Vermilion {

// Floating point type.
#ifndef USEDOUBLEPREC
    typedef float FLOAT;
#else
    typedef double FLOAT;
#endif

// This file contains the code for any engine types
// Namely multidimensional floats

    struct float2 {
        // Data
        FLOAT x, y;

        // Constructor
        float2(FLOAT nx, FLOAT ny) : x(nx), y(ny) { }

        float2() { }

        /// Operators
        // Addition
        float2 operator+(float2 &rhs) const {
            return float2(this->x + rhs.x, this->y + rhs.y);
        }

        // Subtraction
        float2 operator-(float2 &rhs) const {
            return float2(this->x - rhs.x, this->y - rhs.y);
        }

        // Mul
        float2 operator*(float2 &rhs) const {
            return float2(this->x * rhs.x, this->y * rhs.y);
        }

        // Div
        float2 operator/(float2 &rhs) const {
            return float2(this->x / rhs.x, this->y / rhs.y);
        }

        /// Self mods
        // Addition
        float2 &operator+=(float2 &rhs) {
            this->x += rhs.x;
            this->y += rhs.y;
            return *this;
        }

        // Subtraction
        float2 &operator-=(float2 &rhs) {
            this->x -= rhs.x;
            this->y -= rhs.y;
            return *this;
        }

        // Mul
        float2 &operator*=(float2 &rhs) {
            this->x *= rhs.x;
            this->y *= rhs.y;
            return *this;
        }

        // Div
        float2 &operator/=(float2 &rhs) {
            this->x /= rhs.x;
            this->y /= rhs.y;
            return *this;
        }

        // Assignment
        bool operator==(float2 &rhs) const {
            return this->x == rhs.x && this->y == rhs.y;
        }
    };

    struct float3 {
        // Data
        FLOAT x, y, z;

        // Constructor
        float3(FLOAT nx, FLOAT ny, FLOAT nz) : x(nx), y(ny), z(nz) { }

        float3() { }

        /// Operators
        // Addition
        float3 operator+(float3 &rhs) const {
            return float3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
        }

        // FAddition
        friend float3 operator-(float3 &lhs, float3 &rhs) {
            return float3(lhs.x - rhs.x,
                          lhs.y - rhs.y,
                          lhs.z - rhs.z);
        }
        // Subtraction
        float3 operator-(float3 &rhs) const {
            return float3(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
        }

        // Mul
        float3 operator*(float3 &rhs) const {
            return float3(this->x * rhs.x, this->y * rhs.y, this->z * rhs.z);
        }

        // Mul
        float3 operator*(FLOAT &rhs) const {
            return float3(this->x * rhs, this->y * rhs, this->z * rhs);
        }

        // Div
        float3 operator/(float3 &rhs) const {
            return float3(this->x / rhs.x, this->y / rhs.y, this->z / rhs.z);
        }

        /// Self mods
        // Addition
        float3 &operator+=(float3 &rhs) {
            this->x += rhs.x;
            this->y += rhs.y;
            this->z += rhs.z;
            return *this;
        }

        // Subtraction
        float3 &operator-=(float3 &rhs) {
            this->x -= rhs.x;
            this->y -= rhs.y;
            this->z -= rhs.z;
            return *this;
        }

        // Subtraction
        float3 &operator-(FLOAT &rhs) {
            this->x -= rhs;
            this->y -= rhs;
            this->z -= rhs;
            return *this;
        }

        // Mul
        float3 &operator*=(float3 &rhs) {
            this->x *= rhs.x;
            this->y *= rhs.y;
            this->z *= rhs.z;
            return *this;
        }

        // Div
        float3 &operator/=(float3 &rhs) {
            this->x /= rhs.x;
            this->y /= rhs.y;
            this->z /= rhs.z;
            return *this;
        }

        // Assignment
        bool operator==(float3 &rhs) const {
            return this->x == rhs.x && this->y == rhs.y && this->z == rhs.z;
        }
    };

    struct float4 {
        // Data
        FLOAT x, y, z, w;

        // Constructor
        float4(FLOAT nx, FLOAT ny, FLOAT nz, FLOAT nw) : x(nx), y(ny), z(nz), w(nw) { }

        float4() { }

        /// Operators
        // Addition
        float4 operator+(const float4 &rhs) const {
            return float4(this->x + rhs.x,
                          this->y + rhs.y,
                          this->z + rhs.z,
                          this->w + rhs.w);
        }

        // Subtraction
        float4 operator-(float4 &rhs) const {
            return float4(this->x - rhs.x,
                          this->y - rhs.y,
                          this->z - rhs.z,
                          this->w - rhs.w);
        }

        // Mul
        float4 operator*(const float4 &rhs) const {
            return float4(this->x * rhs.x,
                          this->y * rhs.y,
                          this->z * rhs.z,
                          this->w * rhs.w);
        }

        float4 operator*(const float &rhs) const {
            return float4(this->x * rhs,
            this->y * rhs,
            this->z * rhs,
            this->w * rhs
            );
        }

        // Div
        float4 operator/(const float4 &rhs) const {
            return float4(this->x / rhs.x,
                          this->y / rhs.y,
                          this->z / rhs.z,
                          this->w / rhs.w);
        }

        float4 operator/(const uint32_t &rhs) const {
            return float4(this->x / rhs,
                          this->y / rhs,
                          this->z / rhs,
                          this->w / rhs);
        }

        // FAddition
        friend float4 operator+(float4 &lhs, float4 &rhs) {
            return float4(lhs.x + rhs.x,
                          lhs.y + rhs.y,
                          lhs.z + rhs.z,
                          lhs.w + rhs.w);
        }

        /// Self mods
        // Addition
        float4 &operator+=(const float4 &rhs) {
            this->x += rhs.x;
            this->y += rhs.y;
            this->z += rhs.z;
            this->w += rhs.w;
            return *this;
        }

        // Subtraction
        float4 &operator-=(const float4 &rhs) {
            this->x -= rhs.x;
            this->y -= rhs.y;
            this->z -= rhs.z;
            this->w -= rhs.w;
            return *this;
        }

        // Mul
        float4 &operator*=(float4 &rhs) {
            this->x *= rhs.x;
            this->y *= rhs.y;
            this->z *= rhs.z;
            this->w *= rhs.w;
            return *this;
        }

        // Div
        float4 &operator/=(const float4 &rhs) {
            this->x /= rhs.x;
            this->y /= rhs.y;
            this->z /= rhs.z;
            this->w /= rhs.w;
            return *this;
        }

        float4 &operator/=(const uint32_t &rhs) {
            this->x /= rhs;
            this->y /= rhs;
            this->z /= rhs;
            this->w /= rhs;
            return *this;
        }

        // Assignment
        bool operator==(float4 &rhs) const {
            return this->x == rhs.x &&
                   this->y == rhs.y &&
                   this->z == rhs.z &&
                   this->w == rhs.w;
        }
    };

}
#endif //VERMILION_TYPES_H

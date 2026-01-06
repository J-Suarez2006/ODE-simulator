#ifndef MATH_HPP
#define MATH_HPP

#include <SDL3/SDL.h>

#include <array>
#include <initializer_list>
#include <cmath>
#include <iostream>

class Vector2
{
    public:
    Vector2() : v{0, 0} {}
    Vector2(float i, float j) : v{{i, j}} {}

    inline float x() const {return v[0];}
    inline float y() const {return v[1];}

    inline Vector2 operator-() const
    {
        return Vector2(
            -v[0],
            -v[1]
        );
    }

    inline Vector2 operator+(const Vector2& other)
    {
        return Vector2(
            x() + other.x(),
            y() + other.y()
        );
    }

    inline Vector2 operator-(const Vector2& other)
    {
        return *this + (-other);
    }

    inline Vector2 operator*(float scalar)
    {
        return Vector2(
            scalar * x(),
            scalar * y()
        );
    }

    inline Vector2 operator/(float scalar)
    {
        return *this * (1 / scalar);
    }

    Vector2& operator+=(const Vector2& other)
    {
        v[0] += other.x();
        v[1] += other.y();
        return *this;
    }

    private:
    std::array<float, 2> v;
};

inline std::ostream& operator<<(std::ostream& os, const Vector2& v)
{
    os << "[ " << v.x() << ", " << v.y() << " ]" << std::endl;
    return os;
}

class Vector3
{
    public:
    Vector3() : v{0.0f, 0.0f, 0.0f} {}
    Vector3(float i, float j, float k) : v{{i, j, k}} {}

    inline float x() const {return v[0];}
    inline float y() const {return v[1];}
    inline float z() const {return v[2];}

    inline float r() const {return v[0];}
    inline float g() const {return v[1];}
    inline float b() const {return v[2];}

    // Setters
    inline void setX(float new_x) {v[0] = new_x;}
    inline void setY(float new_y) {v[1] = new_y;}
    inline void setZ(float new_z) {v[2] = new_z;}

    inline float magnitude() const
    {
        return std::sqrt(std::pow(x(), 2) + std::pow(y(), 2) + std::pow(z(), 2));
    }

    inline Vector3 unit() const
    {
        float m {magnitude()};
        return Vector3(
            x() / m,
            y() / m,
            z() / m
        );
    }

    inline std::array<float, 4> V4()
    {
        return {
            x(),
            y(),
            z(),
            1.0f
        };
    }

    inline Vector3 cross(const Vector3& other)
    {
        return Vector3(
            y() * other.z() - z() * other.y(),
            z() * other.x() - x() * other.z(),
            x() * other.y() - y() * other.x()
        );
    }

    inline Vector3 operator-() const
    {
        return Vector3(
            -v[0],
            -v[1],
            -v[2]
        );
    }

    inline Vector3 operator+(const Vector3& other)
    {
        return Vector3(
            x() + other.x(),
            y() + other.y(),
            z() + other.z()
        );
    }

    inline Vector3 operator-(const Vector3& other)
    {
        return *this + (-other);
    }

    inline Vector3 operator*(float scalar)
    {
        return Vector3(
            x() * scalar,
            y() * scalar,
            z() * scalar
        );
    }

    inline Vector3 operator/(float scalar)
    {
        return *this * (1.0f / scalar);
    }

    Vector3& operator+=(const Vector3& other)
    {
        v[0] += other.x();
        v[1] += other.y();
        v[2] += other.z();
        return *this;
    }

    Vector3& operator-=(const Vector3& other)
    {
        *this += (-other);
        return *this;
    }

    std::array<float, 3> v;
};

inline float dot(const Vector3& u, const Vector3& v)
{
    return (
        u.x() * v.x() +
        u.y() * v.y() + 
        u.z() * v.z()
    );
}

inline std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    os << "[ " << v.x() << ", " << v.y() << ", " << v.z() << "]" << std::endl;
    return os;
}

using ColorRGB = Vector3;

class Matrix4x4
{
    public:
    using row = std::array<float, 4>;
    std::array<row, 4> m;

    Matrix4x4() {
        for (auto& r : m)
        {
            r.fill(0.0f);
        }
    }
    
    Matrix4x4(std::initializer_list<row> rows) {
        int i = 0;
        for (auto& r : rows)
        {
            m[i++] = r;
        }
    }

    inline Vector3 MatMult(Vector3 v);
    inline Matrix4x4 MatMult(Matrix4x4 other);
};

inline Vector3 Matrix4x4::MatMult(Vector3 v)
{
    std::array<float, 4> v4 = v.V4();
    Vector3 result;

    for (int i = 0; i < 3; ++i)
    {
        float to_add {0.0f};
        for (int j = 0; j < 4; ++j)
        {
            to_add += m[i][j] * v4[j];
        }
        result.v[i] = to_add;
    }

    return result;
}

inline Matrix4x4 Matrix4x4::MatMult(Matrix4x4 other)
{
    Matrix4x4 result {};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j) {
            result.m[i][j] = 0;
            for (int k = 0; k < 4; ++k)
                result.m[i][j] += m[i][k] * other.m[k][j];
        }
    return result;  
}

inline Matrix4x4::row operator+(Matrix4x4::row r1, Matrix4x4::row r2)
{
    Matrix4x4::row res;

    for (int i = 0; i < 4; ++i)
    {
        res[i] = r1[i] + r2[i];
    }

    return res;
}

inline Matrix4x4::row operator-(Matrix4x4::row r)
{
    Matrix4x4::row res;

    for (int i = 0; i < 4; ++i)
    {
        res[i] = -r[i];
    }

    return res;
}

inline Matrix4x4::row operator-(Matrix4x4::row r1, Matrix4x4::row r2)
{
    return r1 + (-r2);
}

inline Matrix4x4::row operator*(Matrix4x4::row r, float scalar)
{
    Matrix4x4::row res;

    for (int i = 0; i < 4; ++i)
    {
        res[i] = scalar * r[i];
    }

    return res;
}

inline Matrix4x4::row operator/(Matrix4x4::row r, float scalar)
{
    return r * (1 / scalar);
}

inline Matrix4x4::row operator+=(Matrix4x4::row& r1, Matrix4x4::row r2)
{
    return r1 = r1 + r2;
}

inline Matrix4x4::row operator-=(Matrix4x4::row& r1, Matrix4x4::row r2)
{
    return r1 = r1 - r2;
}

inline Matrix4x4::row operator*=(Matrix4x4::row& r1, float scalar)
{
    return {
        r1[0] *= scalar,
        r1[1] *= scalar,
        r1[2] *= scalar,
        r1[3] *= scalar
    };
}

inline Matrix4x4::row operator/=(Matrix4x4::row& r1, float scalar)
{
    return r1 *= (1 / scalar);
}

inline Matrix4x4 identity()
{
    return Matrix4x4{
        {1.0f, 0.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 0.0f, 1.0f}
    };
}

inline Matrix4x4 getInverse(Matrix4x4 matrix)
{
    auto& mat = matrix.m;
    Matrix4x4 I = identity();

    for (int r = 0; r < mat.size(); ++r)
    {
        for (int i = r; i < mat.size(); ++i)
        {
            float factor = 1 / mat[i][r];

            mat[i] *= factor;
            I.m[i] *= factor;
        }

        for (int i = r + 1; i < mat.size(); ++i)
        {
            mat[i] -= mat[r];
            I.m[i] -= I.m[r];
        }
    }

    for (int r = mat.size() - 1; r >= 0; --r)
    {
        for (int i = r - 1; i >= 0; --i)
        {
            float factor = mat[i][r];

            mat[i] -= (mat[r] * factor);
            I.m[i] -= (I.m[r] * factor);
        }
    }

    return I;
}

inline std::ostream& operator<<(std::ostream& os, const Matrix4x4& m)
{
    for (auto row : m.m)
    {
        os << "[";
        for (float f : row)
        {
            os << f << ", ";
        }
        os << "]" << std::endl;
    }

    return os;
}

class Quaternion
{
    public:
    Quaternion() : scalar(0), vector() {}
    Quaternion(float w, float x, float y, float z)
    : scalar(w), vector(x, y, z) {}
    Quaternion(float w, const Vector3& v)
    : scalar(w), vector(v) {}

    static inline Quaternion identity()
    {
        return Quaternion(1, 0, 0, 0);
    }

    static inline Quaternion fromAxisAngle(const Vector3& axis, float angle)
    {
        constexpr float PI = 3.14159263f;

        float angle_rad = angle * PI / 180.0f;

        Quaternion q {
            std::cos(angle_rad / 2),
            axis.unit() * std::sin(angle_rad / 2)
        };

        return q.normalize();
    }


    inline float w() const {return scalar;}
    inline float x() const {return vector.x();}
    inline float y() const {return vector.y();}
    inline float z() const {return vector.z();}

    inline float s() const {return scalar;}
    inline Vector3 v() const {return vector;}

    inline Quaternion conjugate() const;
    inline Quaternion normalize() const;

    inline Quaternion operator-() const
    {
        // Negation
        return Quaternion(
            -s(),
            -v()
        );
    }

    private:
    float scalar;
    Vector3 vector;
};

inline float dot(const Quaternion& q1, const Quaternion& q2);
inline float magnitude(const Quaternion& q);

//Matrix4x4 getRotationMatrix(const Quaternion& q);

inline std::ostream& operator<<(std::ostream& o, Quaternion q)
{
    o << "[ " <<
    q.w() << ", " << q.x() << "i, " << q.y() << "j, " << q.z() << "k ]";
    return o;
}

inline Quaternion operator+(const Quaternion& q1, const Quaternion& q2)
{
    // Element-wise quaternion addition
    return Quaternion(
        q1.w() + q2.w(),
        q1.x() + q2.x(),
        q1.y() + q2.y(),
        q1.z() + q2.z()
    );
}

inline Quaternion operator-(const Quaternion& q1, const Quaternion& q2)
{
    // Element-wise quaternion subtraction
    return q1 + (-q2);
}

inline Quaternion operator*(float t, const Quaternion& q)
{
    // Scalar multiplication
    return Quaternion(
        t * q.w(),
        t * q.x(),
        t * q.y(),
        t * q.z()
    );
}

inline Quaternion operator*(const Quaternion& q, float t)
{
    // Scalar multiplication
    return t * q;
}

inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
    // Quaternion multiplication
    return Quaternion(
        q1.s() * q2.s() - dot(q1.v(), q2.v()),
        q2.v() * q1.s() + q1.v() * q2.s() + q1.v().cross(q2.v())
    );
}

inline Quaternion inverse(const Quaternion& q)
{
    return q.conjugate() * (1 / dot(q, q));
}

inline float dot(const Quaternion& q1, const Quaternion& q2)
{
    return (q1.s() * q2.s() + dot(q1.v(), q2.v()));
}

inline float magnitude(const Quaternion& q)
{
    return std::sqrt(dot(q, q));
}

inline Quaternion Quaternion::conjugate() const
{
    return Quaternion(
        s(), -v()
    );
}

inline Quaternion Quaternion::normalize() const
{
    float m = magnitude(*this);
    if (m == 0.0f) return *this;

    return (1.0f / m) * (*this);
}

inline Vector3 rotate(const Vector3& rotate_me, const Vector3& axis, float angle)
{
    constexpr float PI = 3.14159263f;

    Quaternion pure {0, rotate_me};
    float angle_rad = angle * PI / 180.0f;

    Quaternion q {
        std::cos(angle_rad / 2),
        axis.unit() * std::sin(angle_rad / 2)
    };

    q = q.normalize();

    return (q * pure * q.conjugate()).v();
}

inline Vector3 rotate(const Vector3& rotate_me, const Quaternion& q)
{
    Quaternion pure {0, rotate_me};
    return (q * pure * q.conjugate()).v();
}

#endif
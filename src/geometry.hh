#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <iostream>
#include <limits>
#include <algorithm>
#include <cmath>

#include "ver.hh"

// TODO: explicar un poco
//
// Allowed vector operations:
// Point + Direction = Point
// Point - Direction = Point
// Point - Point     = Direction
//
// Direction + Direction       = Direction
// Direction - Direction       = Direction
// Direction * scalar          = Direction
// Direction / scalar          = Direction
// Direction `dot` Direction   = scalar
// Direction `cross` Direction = Direction
// Direction `norm`            = scalar
// Direacton `normalize`       = Direction

class Vec2 { // TODO POINT2 DIRECTION2?
  public:
    Float x, y;

  public:
    bool hasNaNs() const { return std::isnan(x) || std::isnan(y); }

    Vec2() : x{0}, y{0} { assert(!hasNaNs(), "Has NaNs"); }
    Vec2(Float x_, Float y_) : x{x_}, y{y_} { assert(!hasNaNs(), "Has NaNs"); }

    [[nodiscard]] Vec2 operator +(const Vec2 &rhs) const { return Vec2(x + rhs.x, y + rhs.y); }
    [[nodiscard]] Vec2 operator -(const Vec2 &rhs) const { return Vec2(x - rhs.x, y - rhs.y); }

    [[nodiscard]] Vec2 operator *(Float value) const { return Vec2(x + value, y + value); }

    [[nodiscard]] Float &operator[](unsigned int idx) {
      assert(idx < 2, "Index out of bounds(0 <= idx < 3)");
      switch (idx) {
        case 0: return x;
        case 1:
        default:return y;
      }
    }

    [[nodiscard]] Float operator[](unsigned int idx) const {
      assert(idx < 2, "Index out of bounds(0 <= idx < 3)");
      switch (idx) {
        case 0: return x;
        case 1:
        default:return y;
      }
    }

    friend std::ostream &operator <<(std::ostream &os, const Vec2 &v) {
      os << "(" << v.x << ", " << v.y << ")";
      return os;
    }
};

class Vec3 {
  public:
    Float x, y, z;

  public:
    bool hasNaNs() const { return std::isnan(x) || std::isnan(y) || std::isnan(z); }

    Vec3() : x{0}, y{0}, z{0} { assert(!hasNaNs(), "Has NaNs"); }
    Vec3(Float x_, Float y_, Float z_) : x{x_}, y{y_}, z{z_} { assert(!hasNaNs(), "Has NaNs"); }

    [[nodiscard]] Float min() const { return std::min(x, std::min(y, z)); }
    [[nodiscard]] Float max() const { return std::max(x, std::max(y, z)); }
    [[nodiscard]] unsigned int argmax() const { return (x > y) ? ((x > z) ? 0 : 2) : ((y > z) ? 1 : 2); }

    [[nodiscard]] Float &operator[](unsigned int idx) {
      assert(idx < 3, "Index out of bounds(0 <= idx < 3)");
      switch (idx) {
        case 0: return x;
        case 1: return y;
        case 2:
        default: return z;
      }
    }

    [[nodiscard]] Float operator[](unsigned int idx) const {
      assert(idx < 3, "Index out of bounds(0 <= idx < 3)");
      switch (idx) {
        case 0: return x;
        case 1: return y;
        case 2:
        default: return z;
      }
    }

    friend std::ostream &operator <<(std::ostream &os, const Vec3 &v) {
      os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
      return os;
    }
};

class Direction : public Vec3 {
  public:
    using Vec3::Vec3;
    explicit Direction(const Vec3 &vec) : Vec3::Vec3(vec) {}

    [[nodiscard]] bool operator ==(const Direction &v) const { return x == v.x && y == v.y && z == v.z; }
    [[nodiscard]] bool operator !=(const Direction &v) const { return x != v.x || y != v.y || z != v.z; }

    [[nodiscard]] Direction operator -() const { return Direction(-x, -y, -z); }

    [[nodiscard]] Direction operator +(const Direction &rhs) const { return Direction(x + rhs.x, y + rhs.y, z + rhs.z); }
    [[nodiscard]] Direction operator -(const Direction &rhs) const { return Direction(x - rhs.x, y - rhs.y, z - rhs.z); }
    [[nodiscard]] Direction operator *(Float value) const { return Direction(x * value, y * value, z * value); }
    [[nodiscard]] Direction operator /(Float value) const {
      assert(value != 0, "Cannot devide by 0!");
      return (*this) * (1 / value);
    }

    [[nodiscard]] Direction operator *(const Direction &rhs) const { return Direction(x * rhs.x, y * rhs.y, z * rhs.z); }

    Direction &operator +=(const Direction &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;
      assert(!hasNaNs(), "Has NaNs");
      return (*this);
    }
    Direction &operator -=(const Direction &rhs) { return (*this) += (-rhs); }

    Direction &operator *=(Float value) {
      x *= value;
      y *= value;
      z *= value;
      assert(!hasNaNs(), "Has NaNs");
      return (*this);
    }
    
    Direction &operator /=(Float value) {
      assert(value != 0, "Cannot devide by 0!");
      return (*this) *= (1 / value);
    }

    Direction &operator *=(const Direction &rhs) {
      x *= rhs.x;
      y *= rhs.y;
      z *= rhs.z;
      assert(!hasNaNs(), "Has NaNs");
      return (*this);
    }

    [[nodiscard]] Float sqrNorm() const { return x * x + y * y + z * z; }
    [[nodiscard]] Float norm() const { return std::sqrt(sqrNorm()); }
    [[nodiscard]] Direction normalize() const { return (*this) / norm(); }

    [[nodiscard]] Direction abs() const { return Direction(std::abs(x), std::abs(y), std::abs(z)); }
    [[nodiscard]] Direction permute(uint dim1, uint dim2, uint dim3) const { return Direction((*this)[dim1], (*this)[dim2], (*this)[dim3]); }

    [[nodiscard]] Float dot(const Direction &rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }
    [[nodiscard]] Direction cross(const Direction &rhs) const {
      return Direction(y * rhs.z - z * rhs.y,
                       z * rhs.x - x * rhs.z,
                       x * rhs.y - y * rhs.x);
    }

    friend std::ostream &operator <<(std::ostream &os, const Direction &v) {
      os << "Direction";
      os << static_cast<const Vec3&>(v);
      return os;
    }
};

class Point : public Vec3 {
  public:
    using Vec3::Vec3;
    explicit Point(const Vec3 &vec) : Vec3::Vec3(vec) {}

    [[nodiscard]] bool operator ==(const Point &v) const { return x == v.x && y == v.y && z == v.z; }
    [[nodiscard]] bool operator !=(const Point &v) const { return x != v.x || y != v.y || z != v.z; }

    [[nodiscard]] Point operator +(const Direction &rhs) const { return Point(x + rhs.x, y + rhs.y, z + rhs.z); }
    [[nodiscard]] Point operator -(const Direction &rhs) const { return Point(x - rhs.x, y - rhs.y, z - rhs.z); }
    [[nodiscard]] Direction operator -(const Point &rhs) const { return Direction(x - rhs.x, y - rhs.y, z - rhs.z); }

    Point &operator +=(const Direction &rhs) {
      x += rhs.x;
      y += rhs.y;
      z += rhs.z;
      assert(!hasNaNs(), "Has NaNs");
      return (*this);
    }

    Point &operator -=(const Direction &rhs) {
      x -= rhs.x;
      y -= rhs.y;
      z -= rhs.z;
      assert(!hasNaNs(), "Has NaNs");
      return (*this);
    }

    [[nodiscard]] Point abs() const { return Point(std::abs(x), std::abs(y), std::abs(z)); }
    [[nodiscard]] Point permute(uint dim1, uint dim2, uint dim3) const { return Point((*this)[dim1], (*this)[dim2], (*this)[dim3]); }

    friend std::ostream &operator <<(std::ostream &os, const Point &v) {
      os << "Point";
      os << static_cast<const Vec3&>(v);
      return os;
    }
};

class Mat4 {
  public:
    bool hasNaNs() const {
      for (int i = 0; i < 4 * 4; i++)
        if (std::isnan(data[i]))
          return true;
      return false;
    }

    Mat4() : data{} { assert(!hasNaNs(), "Has NaNs"); }
    explicit Mat4(Float diag) : data{} {
      for (int i = 0; i < 4; i ++)
        data[i*4 + i] = diag;

      assert(!hasNaNs(), "Has NaNs");
    }

    Mat4(const Direction &b1, const Direction &b2, const Direction &b3)
      // : data{b1.x, b1.y, b1.z, 0, b2.x, b2.y, b2.z, 0, b3.x, b3.y, b3.z, 0, 0, 0, 0, 1} {}
      : data{b1.x, b2.x, b3.x, 0, b1.y, b2.y, b3.y, 0, b1.z, b2.z, b3.z, 0, 0, 0, 0, 1} {}

    [[nodiscard]] static Mat4 identity() { return Mat4(1); }
    [[nodiscard]] static Mat4 translation(Float dx, Float dy, Float dz) {
      Mat4 mat = Mat4::identity();
      mat.data[0*4 + 3] = dx;
      mat.data[1*4 + 3] = dy;
      mat.data[2*4 + 3] = dz;
      assert(!mat.hasNaNs(), "Has NaNs");
      return mat;
    }
    [[nodiscard]] static Mat4 translation(const Direction &delta) { return Mat4::translation(delta.x, delta.y, delta.z); }
    [[nodiscard]] static Mat4 scale(Float x, Float y, Float z) {
      Mat4 mat = Mat4::identity();
      mat.data[0*4 + 0] = x;
      mat.data[1*4 + 1] = y;
      mat.data[2*4 + 2] = z;
      assert(!mat.hasNaNs(), "Has NaNs");
      return mat;
    }
    [[nodiscard]] static Mat4 scale(const Direction &scaling) { return Mat4::scale(scaling.x, scaling.y, scaling.z); }
    [[nodiscard]] static Mat4 rotate(Float theta, const Direction &axis) {
      Mat4 mat;

      Direction a = axis.normalize();
      Float sinTheta = std::sin(theta);
      Float cosTheta = std::cos(theta);

      mat.data[0*4 + 0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
      mat.data[0*4 + 1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
      mat.data[0*4 + 2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
      mat.data[0*4 + 3] = 0;

      mat.data[1*4 + 0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
      mat.data[1*4 + 1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
      mat.data[1*4 + 2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
      mat.data[1*4 + 3] = 0;

      mat.data[2*4 + 0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
      mat.data[2*4 + 1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
      mat.data[2*4 + 2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
      mat.data[2*4 + 3] = 0;

      assert(!mat.hasNaNs(), "Has NaNs");
      return mat;
    }
    [[nodiscard]] static Mat4 rotate(Float theta, Float x, Float y, Float z) { return Mat4::rotate(theta, Direction(x, y, z)); }

    // [[nodiscard]] constexpr Mat4 operator +(const Mat4 &rhs) const {}
    // [[nodiscard]] constexpr Mat4 operator -(const Mat4 &rhs) const {}

    [[nodiscard]] Mat4 operator *(const Mat4 &rhs) const {
      Mat4 res;
      for (int i = 0; i < 4; i++)
        for (int k = 0; k < 4; k++)
          for (int j = 0; j < 4; j++)
            res.data[i*4 + j] += data[i*4 + k] * rhs.data[k*4 + j];

      assert(!res.hasNaNs(), "Has NaNs");
      return res;
    }

    [[nodiscard]] Point operator *(const Point &rhs) const {
      Float out[4] = {0};
      for (int i = 0; i < 4; i++) { // TODO: tengo dudas
        out[i] = data[i*4 + 0] * rhs.x +
                 data[i*4 + 1] * rhs.y +
                 data[i*4 + 2] * rhs.z +
                 data[i*4 + 3] * 1;
      }
      return Point(out[0], out[1], out[2]);
    }

    [[nodiscard]] Direction operator *(const Direction &rhs) const {
      Float out[4] = {0};
      for (int i = 0; i < 4; i++) { // TODO: tengo dudas
        out[i] = data[i*4 + 0] * rhs.x +
                 data[i*4 + 1] * rhs.y +
                 data[i*4 + 2] * rhs.z +
                 data[i*4 + 3] * 0;
      }
      return Direction(out[0], out[1], out[2]);
    }

    [[nodiscard]] Mat4 invert() const {
      Mat4 inv;
      //if (data[3*4 + 0]==0 && data[3*4 + 1]==0 && data[3*4 + 2]==0 && data[3*4 + 3]==1) {
        //// https://stackoverflow.com/a/155705
        //// This algorithm works only if the matrix has this form:
        //// M = [ux vx wx tx]
        ////     [uy vy wy ty]
        ////     [uz vz wz tz]
        ////     [ 0  0  0  1]
        //// M^-1 = [ux uy uz -dot(u,t)]
        ////        [vx vy vz -dot(v,t)]
        ////        [wx wy wz -dot(w,t)]
        ////        [ 0  0  0     1    ]
        //for (int i = 0; i < 3; i++)
          //for (int j = 0; j < 3; j++) {
            //inv.data[i*4 + j] = data[j*4 + i];
            //inv.data[i*4 + 3] -= data[j*4 + i] * data[j*4 + 3];
          //}
        ////for (int i = 0; i < 3; i++)
          ////inv.data[i * 4 + i] = 1.0 / inv.data[i * 4 + i];
//
        //inv.data[3*4 + 3] = 1;
//
        //assert(
          //(data[0*4+0]*data[1*4+1]*data[2*4+2] +
           //data[0*4+1]*data[1*4+2]*data[2*4+0] +
           //data[0*4+2]*data[1*4+0]*data[2*4+1] -
           //data[0*4+2]*data[1*4+1]*data[2*4+0] -
           //data[0*4+1]*data[1*4+0]*data[2*4+2] -
           //data[0*4+0]*data[1*4+2]*data[2*4+1]) != 0
          //, "Matrix is not invertible!"); // TODO: son todas inversibles????????
//
      //} else
      { // Yoinked from https://gitlab.freedesktop.org/mesa/glu/-/blob/master/src/libutil/project.c#L167
        inv.data[0] =   data[5]*data[10]*data[15] - data[5]*data[11]*data[14] - data[9]*data[6]*data[15]
                      + data[9]*data[7]*data[14] + data[13]*data[6]*data[11] - data[13]*data[7]*data[10];
        inv.data[4] =  -data[4]*data[10]*data[15] + data[4]*data[11]*data[14] + data[8]*data[6]*data[15]
                      - data[8]*data[7]*data[14] - data[12]*data[6]*data[11] + data[12]*data[7]*data[10];
        inv.data[8] =   data[4]*data[9]*data[15] - data[4]*data[11]*data[13] - data[8]*data[5]*data[15]
                      + data[8]*data[7]*data[13] + data[12]*data[5]*data[11] - data[12]*data[7]*data[9];
        inv.data[12] = -data[4]*data[9]*data[14] + data[4]*data[10]*data[13] + data[8]*data[5]*data[14]
                      - data[8]*data[6]*data[13] - data[12]*data[5]*data[10] + data[12]*data[6]*data[9];
        inv.data[1] =  -data[1]*data[10]*data[15] + data[1]*data[11]*data[14] + data[9]*data[2]*data[15]
                      - data[9]*data[3]*data[14] - data[13]*data[2]*data[11] + data[13]*data[3]*data[10];
        inv.data[5] =   data[0]*data[10]*data[15] - data[0]*data[11]*data[14] - data[8]*data[2]*data[15]
                      + data[8]*data[3]*data[14] + data[12]*data[2]*data[11] - data[12]*data[3]*data[10];
        inv.data[9] =  -data[0]*data[9]*data[15] + data[0]*data[11]*data[13] + data[8]*data[1]*data[15]
                      - data[8]*data[3]*data[13] - data[12]*data[1]*data[11] + data[12]*data[3]*data[9];
        inv.data[13] =  data[0]*data[9]*data[14] - data[0]*data[10]*data[13] - data[8]*data[1]*data[14]
                      + data[8]*data[2]*data[13] + data[12]*data[1]*data[10] - data[12]*data[2]*data[9];
        inv.data[2] =   data[1]*data[6]*data[15] - data[1]*data[7]*data[14] - data[5]*data[2]*data[15]
                      + data[5]*data[3]*data[14] + data[13]*data[2]*data[7] - data[13]*data[3]*data[6];
        inv.data[6] =  -data[0]*data[6]*data[15] + data[0]*data[7]*data[14] + data[4]*data[2]*data[15]
                      - data[4]*data[3]*data[14] - data[12]*data[2]*data[7] + data[12]*data[3]*data[6];
        inv.data[10] =  data[0]*data[5]*data[15] - data[0]*data[7]*data[13] - data[4]*data[1]*data[15]
                      + data[4]*data[3]*data[13] + data[12]*data[1]*data[7] - data[12]*data[3]*data[5];
        inv.data[14] = -data[0]*data[5]*data[14] + data[0]*data[6]*data[13] + data[4]*data[1]*data[14]
                      - data[4]*data[2]*data[13] - data[12]*data[1]*data[6] + data[12]*data[2]*data[5];
        inv.data[3] =  -data[1]*data[6]*data[11] + data[1]*data[7]*data[10] + data[5]*data[2]*data[11]
                      - data[5]*data[3]*data[10] - data[9]*data[2]*data[7] + data[9]*data[3]*data[6];
        inv.data[7] =   data[0]*data[6]*data[11] - data[0]*data[7]*data[10] - data[4]*data[2]*data[11]
                      + data[4]*data[3]*data[10] + data[8]*data[2]*data[7] - data[8]*data[3]*data[6];
        inv.data[11] = -data[0]*data[5]*data[11] + data[0]*data[7]*data[9] + data[4]*data[1]*data[11]
                      - data[4]*data[3]*data[9] - data[8]*data[1]*data[7] + data[8]*data[3]*data[5];
        inv.data[15] =  data[0]*data[5]*data[10] - data[0]*data[6]*data[9] - data[4]*data[1]*data[10]
                      + data[4]*data[2]*data[9] + data[8]*data[1]*data[6] - data[8]*data[2]*data[5];

        Float det = data[0]*inv.data[0] + data[1]*inv.data[4] + data[2]*inv.data[8] + data[3]*inv.data[12];
        assert(det != 0, "Matrix is not invertible!");

        det = 1.0 / det;
        for (int i = 0; i < 4 * 4; i++)
          inv.data[i] = det * inv.data[i];
      }

      assert(!inv.hasNaNs(), "Has NaNs");
      return inv;
    }

    friend std::ostream &operator <<(std::ostream &os, const Mat4 &mat) {
      for (int i = 0; i < 4; i++) {
        os << "[";
        for (int j = 0; j < 3; j++)
          os << mat.data[i*4 + j] << ", ";
        os << mat.data[i*4 + 3] << "]" << std::endl;
      }
      return os;
    }

  private:
    Float data[4 * 4];
};

class Ray {
  public:
    bool hasNaNs() const { return o.hasNaNs() || d.hasNaNs(); }

    Ray(const Point &origin, const Direction &direction) : o{origin}, d{direction.normalize()} { assert(!hasNaNs(), "Has NaNs"); }

    [[nodiscard]] Point operator()(Float t) const { return o + d * t; }

    friend std::ostream &operator <<(std::ostream &os, const Ray &ray) {
      os << "Ray(" << ray.o << ", " << ray.d << ")";
      return os;
    }

  public:
    Point o;
    Direction d;
    // Float tMax; // TODO
    // Medium medium; // TODO
};

class Bounds {
  public:
    bool hasNaNs() const { return min.hasNaNs() || max.hasNaNs(); }

    Bounds() {
      Float minNum = std::numeric_limits<Float>::lowest();
      Float maxNum = std::numeric_limits<Float>::max();

      this->min = Point(maxNum, maxNum, maxNum);
      this->max = Point(minNum, minNum, minNum);

      assert(!hasNaNs(), "Has NaNs");
    }

    Bounds(const Point &p) : min{p}, max{p} { assert(!hasNaNs(), "Has NaNs"); }

    Bounds(const Point &p0, const Point &p1)
    : min(std::min(p0.x, p1.x), std::min(p0.y, p1.y), std::min(p0.z, p1.z)),
      max(std::max(p0.x, p1.x), std::max(p0.y, p1.y), std::max(p0.z, p1.z))
    { assert(!hasNaNs(), "Has NaNs"); }

    [[nodiscard]] Point &operator[](unsigned int i) {
      assert(i == 0 || i == 1, "Bounds subscript must be 0 or 1");
      return (i == 0) ? min : max;
    }

    [[nodiscard]] Point operator[](unsigned int i) const {
      assert(i == 0 || i == 1, "Bounds subscript must be 0 or 1");
      return (i == 0) ? min : max;
    }

    [[nodiscard]] Point corner(unsigned int corner) const {
      assert(corner < 8, "Corners go from 0 to 8");
      Float x = (corner & 1) ? max.x : min.x;
      Float y = (corner & 2) ? max.y : min.y;
      Float z = (corner & 4) ? max.z : min.z;
      return Point(x, y, z);
    }

    [[nodiscard]] Bounds Union(const Point &p) const {
      return Bounds(Point(std::min(min.x, p.x), std::min(min.y, p.y), std::min(min.z, p.z)),
                    Point(std::max(max.x, p.x), std::max(max.y, p.y), std::max(max.z, p.z)));
    }

    [[nodiscard]] Bounds Union(const Bounds &b) const {
      return Bounds(Point(std::min(min.x, b.min.x), std::min(min.y, b.min.y), std::min(min.z, b.min.z)),
                    Point(std::max(max.x, b.max.x), std::max(max.y, b.max.y), std::max(max.z, b.max.z)));
    }

    [[nodiscard]] Bounds intersection(const Bounds &b) const {
      return Bounds(Point(std::max(min.x, b.min.x), std::max(min.y, b.min.y), std::max(min.z, b.min.z)),
                    Point(std::min(max.x, b.max.x), std::min(max.y, b.max.y), std::min(max.z, b.max.z)));
    }

    [[nodiscard]] bool overlaps(const Bounds &b) const {
      bool x = (max.x >= b.min.x) && (min.x <= b.max.x);
      bool y = (max.y >= b.min.y) && (min.y <= b.max.y);
      bool z = (max.z >= b.min.z) && (min.z <= b.max.z);
      return x && y && z;
    }

    [[nodiscard]] bool contains(const Point &p) const {
      return (p.x >= min.x && p.x <= max.x &&
              p.y >= min.y && p.y <= max.y &&
              p.z >= min.z && p.z <= max.z);
    }

    [[nodiscard]] bool containsExclusive(const Point &p) const {
      return (p.x >= min.x && p.x < max.x &&
              p.y >= min.y && p.y < max.y &&
              p.z >= min.z && p.z < max.z);
    }

    [[nodiscard]] Bounds expand(Float delta) const {
      return Bounds(min - Direction(delta, delta, delta), max + Direction(delta, delta, delta));
    }

    [[nodiscard]] Direction diagonal() const { return max - min; }

    [[nodiscard]] Float surfaceArea() const {
      Direction d = diagonal();
      return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
    }

    [[nodiscard]] Float volume() const {
      Direction d = diagonal();
      return d.x * d.y * d.z;
    }

    [[nodiscard]] uint maximumExtent() const {
      Direction d = diagonal();
      if (d.x > d.y && d.x > d.z) return 0;
      else if (d.y > d.z)         return 1;
      else                        return 2;
    }

    [[nodiscard]] Point lerp(const Point &t) const {
      return Point(::lerp(t.x, min.x, max.x),
                   ::lerp(t.y, min.y, max.y),
                   ::lerp(t.z, min.z, max.z));
    }

    [[nodiscard]] Direction offset(const Point &p) const {
      Direction o = p - min;
      if (max.x > min.x) o.x /= max.x - min.x;
      if (max.y > min.y) o.y /= max.y - min.y;
      if (max.z > min.z) o.z /= max.z - min.z;
      return o;
    }

    [[nodiscard]] bool intersect(const Ray &ray, Float &t0, Float &t1) const {
      t0 = 0; t1 = std::numeric_limits<Float>::max();
      for (int i = 0; i < 3; i++) {
        Float invRayDir = 1.0 / ray.d[i];
        Float tNear = (min[i] - ray.o[i]) * invRayDir;
        Float tFar = (max[i] - ray.o[i]) * invRayDir;

        if (tNear > tFar) std::swap(tNear, tFar);

        tFar *= 1 + 2 * gamma(3);
        t0 = tNear > t0 ? tNear : t0;
        t1 = tFar < t1 ? tFar : t1;
        if (t0 > t1) return false;
      }
      return true;
    }

    [[nodiscard]] bool intersect(const Ray &ray, const Direction &invDir, const int dirIsNeg[3]) const {
      const Float raytMax = std::numeric_limits<Float>::max();
      const Bounds &bounds = *this;

      Float tMin = (bounds[dirIsNeg[0]].x - ray.o.x) * invDir.x;
      Float tMax = (bounds[1 - dirIsNeg[0]].x - ray.o.x) * invDir.x;
      Float tyMin = (bounds[dirIsNeg[1]].y - ray.o.y) * invDir.y;
      Float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.o.y) * invDir.y;

      tMax *= 1 + 2 * gamma(3);
      tyMax *= 1 + 2 * gamma(3);
      if (tMin > tyMax || tyMin > tMax) return false;
      if (tyMin > tMin) tMin = tyMin;
      if (tyMax < tMax) tMax = tyMax;

      Float tzMin = (bounds[dirIsNeg[2]].z - ray.o.z) * invDir.z;
      Float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.o.z) * invDir.z;

      tzMax *= 1 + 2 * gamma(3);
      if (tMin > tzMax || tzMin > tMax) return false;
      if (tzMin > tMin) tMin = tzMin;
      if (tzMax < tMax) tMax = tzMax;

      return (tMin < raytMax) && (tMax > 0);
    }

    friend std::ostream &operator <<(std::ostream &os, const Bounds &b) {
      os << "Bounds(" << b.min << ", " << b.max << ")";
      return os;
    }

  public: //private:
    Point min, max;
};

void makeCoordSystem(const Direction &b1, Direction &b2, Direction &b3);

#endif // GEOMETRY_H_

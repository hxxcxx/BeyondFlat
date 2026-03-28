#include "cagd_types.h"
#include <cmath>

namespace cagd {

Matrix4d perspectiveMatrix(double fovDeg, double aspect, double nearPlane, double farPlane) {
    double fovRad = fovDeg * 3.14159265358979323846 / 180.0;
    double f = 1.0 / std::tan(fovRad / 2.0);

    Matrix4d m = Matrix4d::Zero();
    m(0, 0) = f / aspect;
    m(1, 1) = f;
    m(2, 2) = (farPlane + nearPlane) / (nearPlane - farPlane);
    m(2, 3) = (2.0 * farPlane * nearPlane) / (nearPlane - farPlane);
    m(3, 2) = -1.0;
    return m;
}

Matrix4d orthoMatrix(double left, double right, double bottom, double top, double nearPlane, double farPlane) {
    Matrix4d m = Matrix4d::Zero();
    m(0, 0) = 2.0 / (right - left);
    m(1, 1) = 2.0 / (top - bottom);
    m(2, 2) = -2.0 / (farPlane - nearPlane);
    m(0, 3) = -(right + left) / (right - left);
    m(1, 3) = -(top + bottom) / (top - bottom);
    m(2, 3) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    m(3, 3) = 1.0;
    return m;
}

Matrix4d lookAtMatrix(const Vector3d& eye, const Vector3d& center, const Vector3d& up) {
    Vector3d f = (center - eye).normalized();
    Vector3d r = f.cross(up).normalized();
    Vector3d u = r.cross(f).normalized();

    Matrix4d m = Matrix4d::Identity();
    m(0, 0) = r.x();  m(0, 1) = r.y();  m(0, 2) = r.z();  m(0, 3) = -r.dot(eye);
    m(1, 0) = u.x();  m(1, 1) = u.y();  m(1, 2) = u.z();  m(1, 3) = -u.dot(eye);
    m(2, 0) = -f.x(); m(2, 1) = -f.y(); m(2, 2) = -f.z(); m(2, 3) = f.dot(eye);
    return m;
}

Matrix4d translationMatrix(const Vector3d& offset) {
    Matrix4d m = Matrix4d::Identity();
    m.block<3, 1>(0, 3) = offset;
    return m;
}

Matrix4d rotationXMatrix(double angleRad) {
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);
    Matrix4d m = Matrix4d::Identity();
    m(1, 1) = c;  m(1, 2) = -s;
    m(2, 1) = s;  m(2, 2) = c;
    return m;
}

Matrix4d rotationYMatrix(double angleRad) {
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);
    Matrix4d m = Matrix4d::Identity();
    m(0, 0) = c;  m(0, 2) = s;
    m(2, 0) = -s; m(2, 2) = c;
    return m;
}

Matrix4d rotationZMatrix(double angleRad) {
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);
    Matrix4d m = Matrix4d::Identity();
    m(0, 0) = c;  m(0, 1) = -s;
    m(1, 0) = s;  m(1, 1) = c;
    return m;
}

Matrix4d scaleMatrix(double s) {
    Matrix4d m = Matrix4d::Identity();
    m(0, 0) = s;
    m(1, 1) = s;
    m(2, 2) = s;
    return m;
}

} // namespace cagd

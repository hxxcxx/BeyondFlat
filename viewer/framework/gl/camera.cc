#include "viewer/framework/gl/camera.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace cagd {

OrbitCamera::OrbitCamera()
    : target_(0.0, 0.0, 0.0)
    , yaw_(0.4)
    , pitch_(0.5)
    , distance_(5.0)
    , fovDeg_(45.0)
    , aspect_(16.0 / 9.0)
    , nearPlane_(0.1)
    , farPlane_(100.0)
    , rotateSpeed_(0.005)
    , panSpeed_(0.005)
    , zoomSpeed_(1.0) {}

void OrbitCamera::setTarget(const Vector3d& target) {
    target_ = target;
}

void OrbitCamera::setDistance(double distance) {
    distance_ = std::max(0.1, distance);
}

void OrbitCamera::setYawPitch(double yawDeg, double pitchDeg) {
    yaw_ = yawDeg * M_PI / 180.0;
    pitch_ = pitchDeg * M_PI / 180.0;
    clampPitch();
}

Matrix4d OrbitCamera::viewMatrix() const {
    return lookAtMatrix(eyePosition(), target_, Vector3d::UnitY());
}

Matrix4d OrbitCamera::projectionMatrix() const {
    return perspectiveMatrix(fovDeg_, aspect_, nearPlane_, farPlane_);
}

void OrbitCamera::setPerspective(double fovDeg, double aspect, double nearPlane, double farPlane) {
    fovDeg_ = fovDeg;
    aspect_ = aspect;
    nearPlane_ = nearPlane;
    farPlane_ = farPlane;
}

void OrbitCamera::setAspect(double aspect) {
    aspect_ = aspect;
}

void OrbitCamera::onMouseDragRotate(double deltaX, double deltaY) {
    yaw_ -= deltaX * rotateSpeed_;
    pitch_ += deltaY * rotateSpeed_;
    clampPitch();
}

void OrbitCamera::onMouseDragPan(double deltaX, double deltaY) {
    // Pan in the camera's local X-Y plane
    Vector3d right = Vector3d(std::cos(yaw_), std::sin(yaw_), 0.0).normalized();
    Vector3d up = Vector3d::UnitZ();
    double panScale = distance_ * panSpeed_;

    target_ -= right * deltaX * panScale;
    target_ += up * deltaY * panScale;
}

void OrbitCamera::onMouseScroll(double deltaY) {
    distance_ *= std::pow(1.1, -deltaY * zoomSpeed_);
    distance_ = std::max(0.5, std::min(distance_, 100.0));
}

Vector3d OrbitCamera::eyePosition() const {
    double cp = std::cos(pitch_);
    double sp = std::sin(pitch_);
    double cy = std::cos(yaw_);
    double sy = std::sin(yaw_);

    return target_ + distance_ * Vector3d(cp * cy, cp * sy, sp);
}

void OrbitCamera::clampPitch() {
    const double limit = M_PI / 2.0 - 0.01;
    pitch_ = std::max(-limit, std::min(limit, pitch_));
}

void OrbitCamera::reset() {
    target_ = Vector3d(0.0, 0.0, 0.0);
    yaw_ = 0.4;
    pitch_ = 0.5;
    distance_ = 8.0;
}

} // namespace cagd

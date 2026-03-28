#pragma once

// Orbit camera for 3D scene navigation
// Supports rotation, pan, zoom via mouse interaction

#include "cagd_types.h"

namespace cagd {

class OrbitCamera {
public:
    OrbitCamera();

    // Camera parameters
    void setTarget(const Vector3d& target);
    void setDistance(double distance);
    void setYawPitch(double yawDeg, double pitchDeg);

    // Get view and projection matrices
    Matrix4d viewMatrix() const;
    Matrix4d projectionMatrix() const;

    // Set projection parameters
    void setPerspective(double fovDeg, double aspect, double nearPlane, double farPlane);
    void setAspect(double aspect);

    // Mouse interaction
    void onMouseDragRotate(double deltaX, double deltaY);
    void onMouseDragPan(double deltaX, double deltaY);
    void onMouseScroll(double deltaY);

    // Position queries
    Vector3d eyePosition() const;
    Vector3d target() const { return target_; }
    double distance() const { return distance_; }

    // Reset
    void reset();

private:
    // Orbit parameters
    Vector3d target_;
    double yaw_;       // Horizontal rotation in radians
    double pitch_;     // Vertical rotation in radians
    double distance_;  // Distance from target

    // Projection parameters
    double fovDeg_;
    double aspect_;
    double nearPlane_;
    double farPlane_;

    // Sensitivity
    double rotateSpeed_;
    double panSpeed_;
    double zoomSpeed_;

    // Clamp pitch to avoid gimbal lock
    void clampPitch();
};

} // namespace cagd

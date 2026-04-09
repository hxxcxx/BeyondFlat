// CAGD CLI Managed Wrapper - GL Math utilities
#pragma once

#include "../cagd_cli_types.h"

namespace cagdcli {

    // ========================================================================
    // Math Utilities wrapper
    // ========================================================================

    public ref class GlMath {
    public:
        static Vec3 PerspectiveMatrix(double fovDeg, double aspect, double nearPlane, double farPlane) {
            Eigen::Matrix4d m = cagd::perspectiveMatrix(fovDeg, aspect, nearPlane, farPlane);
            return Vec3(m(0, 3), m(1, 3), m(2, 3));
        }

        static Vec3 OrthoMatrix(double left, double right, double bottom, double top, double nearPlane, double farPlane) {
            Eigen::Matrix4d m = cagd::orthoMatrix(left, right, bottom, top, nearPlane, farPlane);
            return Vec3(m(0, 3), m(1, 3), m(2, 3));
        }

        static array<double>^ LookAtMatrix(Vec3 eye, Vec3 center, Vec3 up) {
            Eigen::Matrix4d m = cagd::lookAtMatrix(eye.ToNative(), center.ToNative(), up.ToNative());
            return EigenToArray(m);
        }

        static array<double>^ TranslationMatrix(Vec3 offset) {
            Eigen::Matrix4d m = cagd::translationMatrix(offset.ToNative());
            return EigenToArray(m);
        }

        static array<double>^ RotationXMatrix(double angleRad) {
            Eigen::Matrix4d m = cagd::rotationXMatrix(angleRad);
            return EigenToArray(m);
        }

        static array<double>^ RotationYMatrix(double angleRad) {
            Eigen::Matrix4d m = cagd::rotationYMatrix(angleRad);
            return EigenToArray(m);
        }

        static array<double>^ RotationZMatrix(double angleRad) {
            Eigen::Matrix4d m = cagd::rotationZMatrix(angleRad);
            return EigenToArray(m);
        }

        static array<double>^ ScaleMatrix(double s) {
            Eigen::Matrix4d m = cagd::scaleMatrix(s);
            return EigenToArray(m);
        }

    private:
        static array<double>^ EigenToArray(const Eigen::Matrix4d& m) {
            array<double>^ arr = gcnew array<double>(16);
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    arr[i * 4 + j] = m(i, j);
            return arr;
        }
    };

} // namespace cagdcli

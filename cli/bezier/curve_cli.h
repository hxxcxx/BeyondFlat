// CAGD CLI Managed Wrapper - Bezier Curves
#pragma once

#include "../cagd_cli_types.h"

namespace cagdcli {

    // ========================================================================
    // Bezier Curve 2D wrapper
    // ========================================================================

    public ref class BezierCurve2d {
    public:
        BezierCurve2d(array<Vec2>^ controlPoints) {
            std::vector<Eigen::Vector2d> nativePoints;
            for (int i = 0; i < controlPoints->Length; i++)
                nativePoints.push_back(controlPoints[i].ToNative());
            _curve = new cagd::BezierCurve2d(nativePoints);
        }

        ~BezierCurve2d() { delete _curve; }

        Vec2 Evaluate(double t) { return Vec2(_curve->evaluate(t)); }
        Vec2 DeCasteljau(double t) { return Vec2(_curve->deCasteljau(t)); }
        Vec2 Derivative(double t, int order = 1) { return Vec2(_curve->derivative(t, order)); }

        property int Degree { int get() { return _curve->degree(); } }

        array<Vec2>^ GetControlPoints() {
            const auto& cp = _curve->controlPoints();
            array<Vec2>^ arr = gcnew array<Vec2>(static_cast<int>(cp.size()));
            for (size_t i = 0; i < cp.size(); i++) arr[i] = Vec2(cp[i]);
            return arr;
        }

        void SetControlPoints(array<Vec2>^ points) {
            std::vector<Eigen::Vector2d> nativePoints;
            for (int i = 0; i < points->Length; i++)
                nativePoints.push_back(points[i].ToNative());
            _curve->setControlPoints(nativePoints);
        }

        array<BezierCurve2d^>^ Subdivide(double t) {
            auto result = _curve->subdivide(t);
            array<BezierCurve2d^>^ arr = gcnew array<BezierCurve2d^>(2);
            arr[0] = gcnew BezierCurve2d(result.first);
            arr[1] = gcnew BezierCurve2d(result.second);
            return arr;
        }

        BezierCurve2d^ ElevateDegree() { return gcnew BezierCurve2d(_curve->elevateDegree()); }
        BezierCurve2d^ ReduceDegree() {
            auto result = _curve->reduceDegree();
            return result ? gcnew BezierCurve2d(result.value()) : nullptr;
        }

    private:
        cagd::BezierCurve2d* _curve;
        BezierCurve2d(const cagd::BezierCurve2d& other) : _curve(new cagd::BezierCurve2d(other.controlPoints())) {}
    };

    // ========================================================================
    // Bezier Curve 3D wrapper
    // ========================================================================

    public ref class BezierCurve3d {
    public:
        BezierCurve3d(array<Vec3>^ controlPoints) {
            std::vector<Eigen::Vector3d> nativePoints;
            for (int i = 0; i < controlPoints->Length; i++)
                nativePoints.push_back(controlPoints[i].ToNative());
            _curve = new cagd::BezierCurve3d(nativePoints);
        }

        ~BezierCurve3d() { delete _curve; }

        Vec3 Evaluate(double t) { return Vec3(_curve->evaluate(t)); }
        Vec3 DeCasteljau(double t) { return Vec3(_curve->deCasteljau(t)); }
        Vec3 Derivative(double t, int order = 1) { return Vec3(_curve->derivative(t, order)); }

        property int Degree { int get() { return _curve->degree(); } }

        array<Vec3>^ GetControlPoints() {
            const auto& cp = _curve->controlPoints();
            array<Vec3>^ arr = gcnew array<Vec3>(static_cast<int>(cp.size()));
            for (size_t i = 0; i < cp.size(); i++) arr[i] = Vec3(cp[i]);
            return arr;
        }

        void SetControlPoints(array<Vec3>^ points) {
            std::vector<Eigen::Vector3d> nativePoints;
            for (int i = 0; i < points->Length; i++)
                nativePoints.push_back(points[i].ToNative());
            _curve->setControlPoints(nativePoints);
        }

        BezierCurve3d^ ElevateDegree() { return gcnew BezierCurve3d(_curve->elevateDegree()); }
        BezierCurve3d^ ReduceDegree() {
            auto result = _curve->reduceDegree();
            return result ? gcnew BezierCurve3d(result.value()) : nullptr;
        }

    private:
        cagd::BezierCurve3d* _curve;
        BezierCurve3d(const cagd::BezierCurve3d& other) : _curve(new cagd::BezierCurve3d(other.controlPoints())) {}
    };

} // namespace cagdcli

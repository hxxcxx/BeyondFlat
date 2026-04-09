// CAGD CLI Managed Wrapper - B-Spline Curves
#pragma once

#include "../cagd_cli_types.h"

namespace cagdcli {

    // ========================================================================
    // B-Spline Curve 2D wrapper
    // ========================================================================

    public ref class BSplineCurve2d {
    public:
        BSplineCurve2d(int degree, array<Vec2>^ controlPoints) {
            std::vector<Eigen::Vector2d> nativePoints;
            for (int i = 0; i < controlPoints->Length; i++)
                nativePoints.push_back(controlPoints[i].ToNative());
            _curve = new cagd::BSplineCurve2d(degree, nativePoints);
        }

        BSplineCurve2d(int degree, array<Vec2>^ controlPoints, array<double>^ knots) {
            std::vector<Eigen::Vector2d> nativePoints;
            for (int i = 0; i < controlPoints->Length; i++)
                nativePoints.push_back(controlPoints[i].ToNative());
            std::vector<double> nativeKnots;
            for (int i = 0; i < knots->Length; i++)
                nativeKnots.push_back(knots[i]);
            _curve = new cagd::BSplineCurve2d(degree, nativePoints, nativeKnots);
        }

        ~BSplineCurve2d() { delete _curve; }

        Vec2 Evaluate(double t) { return Vec2(_curve->evaluate(t)); }
        Vec2 Derivative(double t, int order = 1) { return Vec2(_curve->derivative(t, order)); }

        property int Degree { int get() { return _curve->degree(); } }
        property int ControlPointCount { int get() { return _curve->controlPointCount(); } }
        property int KnotCount { int get() { return _curve->knotCount(); } }

        array<Vec2>^ GetControlPoints() {
            const auto& cp = _curve->controlPoints();
            array<Vec2>^ arr = gcnew array<Vec2>(static_cast<int>(cp.size()));
            for (size_t i = 0; i < cp.size(); i++) arr[i] = Vec2(cp[i]);
            return arr;
        }

        array<double>^ GetKnots() {
            const auto& k = _curve->knots();
            array<double>^ arr = gcnew array<double>(static_cast<int>(k.size()));
            for (size_t i = 0; i < k.size(); i++) arr[i] = k[i];
            return arr;
        }

        void SetKnots(array<double>^ knots) {
            std::vector<double> nativeKnots;
            for (int i = 0; i < knots->Length; i++)
                nativeKnots.push_back(knots[i]);
            _curve->setKnots(nativeKnots);
        }

        array<Vec2>^ InsertKnot(double t, int multiplicity = 1) {
            auto result = _curve->insertKnot(t, multiplicity);
            array<Vec2>^ arr = gcnew array<Vec2>(static_cast<int>(result.size()));
            for (size_t i = 0; i < result.size(); i++) arr[i] = Vec2(result[i]);
            return arr;
        }

        property double DomainMin { double get() { return _curve->domain().first; } }
        property double DomainMax { double get() { return _curve->domain().second; } }
        property bool IsValid { bool get() { return _curve->isValid(); } }

    private:
        cagd::BSplineCurve2d* _curve;
    };

    // ========================================================================
    // B-Spline Curve 3D wrapper
    // ========================================================================

    public ref class BSplineCurve3d {
    public:
        BSplineCurve3d(int degree, array<Vec3>^ controlPoints) {
            std::vector<Eigen::Vector3d> nativePoints;
            for (int i = 0; i < controlPoints->Length; i++)
                nativePoints.push_back(controlPoints[i].ToNative());
            _curve = new cagd::BSplineCurve3d(degree, nativePoints);
        }

        BSplineCurve3d(int degree, array<Vec3>^ controlPoints, array<double>^ knots) {
            std::vector<Eigen::Vector3d> nativePoints;
            for (int i = 0; i < controlPoints->Length; i++)
                nativePoints.push_back(controlPoints[i].ToNative());
            std::vector<double> nativeKnots;
            for (int i = 0; i < knots->Length; i++)
                nativeKnots.push_back(knots[i]);
            _curve = new cagd::BSplineCurve3d(degree, nativePoints, nativeKnots);
        }

        ~BSplineCurve3d() { delete _curve; }

        Vec3 Evaluate(double t) { return Vec3(_curve->evaluate(t)); }
        Vec3 Derivative(double t, int order = 1) { return Vec3(_curve->derivative(t, order)); }

        property int Degree { int get() { return _curve->degree(); } }
        property int ControlPointCount { int get() { return _curve->controlPointCount(); } }
        property int KnotCount { int get() { return _curve->knotCount(); } }

        array<Vec3>^ GetControlPoints() {
            const auto& cp = _curve->controlPoints();
            array<Vec3>^ arr = gcnew array<Vec3>(static_cast<int>(cp.size()));
            for (size_t i = 0; i < cp.size(); i++) arr[i] = Vec3(cp[i]);
            return arr;
        }

        array<double>^ GetKnots() {
            const auto& k = _curve->knots();
            array<double>^ arr = gcnew array<double>(static_cast<int>(k.size()));
            for (size_t i = 0; i < k.size(); i++) arr[i] = k[i];
            return arr;
        }

        void SetKnots(array<double>^ knots) {
            std::vector<double> nativeKnots;
            for (int i = 0; i < knots->Length; i++)
                nativeKnots.push_back(knots[i]);
            _curve->setKnots(nativeKnots);
        }

        array<Vec3>^ InsertKnot(double t, int multiplicity = 1) {
            auto result = _curve->insertKnot(t, multiplicity);
            array<Vec3>^ arr = gcnew array<Vec3>(static_cast<int>(result.size()));
            for (size_t i = 0; i < result.size(); i++) arr[i] = Vec3(result[i]);
            return arr;
        }

        property double DomainMin { double get() { return _curve->domain().first; } }
        property double DomainMax { double get() { return _curve->domain().second; } }
        property bool IsValid { bool get() { return _curve->isValid(); } }

    private:
        cagd::BSplineCurve3d* _curve;
    };

} // namespace cagdcli

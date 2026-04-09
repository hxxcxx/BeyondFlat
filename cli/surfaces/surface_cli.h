// CAGD CLI Managed Wrapper - Bezier Surface
#pragma once

#include "../cagd_cli_types.h"

namespace cagdcli {

    // ========================================================================
    // Bezier Surface wrapper
    // ========================================================================

    public ref class BezierSurface {
    public:
        BezierSurface(array<array<Vec3>^>^ controlPoints) {
            cagd::SurfacePointGrid3d nativeGrid;
            for (int i = 0; i < controlPoints->Length; i++) {
                std::vector<Eigen::Vector3d> row;
                for (int j = 0; j < controlPoints[i]->Length; j++)
                    row.push_back(controlPoints[i][j].ToNative());
                nativeGrid.push_back(row);
            }
            _surface = new cagd::BezierSurface(nativeGrid);
        }

        ~BezierSurface() { delete _surface; }

        Vec3 Evaluate(double u, double v) { return Vec3(_surface->evaluate(u, v)); }
        Vec3 DeCasteljau(double u, double v) { return Vec3(_surface->deCasteljau(u, v)); }

        tuple<Vec3, Vec3>^ Derivatives(double u, double v) {
            auto result = _surface->derivatives(u, v);
            return tuple::Create(Vec3(result.first), Vec3(result.second));
        }

        Vec3 Normal(double u, double v) { return Vec3(_surface->normal(u, v)); }

        property int DegreeU { int get() { return _surface->degreeU(); } }
        property int DegreeV { int get() { return _surface->degreeV(); } }
        property int NumRows { int get() { return _surface->numRows(); } }
        property int NumCols { int get() { return _surface->numCols(); } }

        Vec3 GetControlPoint(int row, int col) { return Vec3(_surface->controlPoint(row, col)); }
        void SetControlPoint(int row, int col, Vec3 point) { _surface->setControlPoint(row, col, point.ToNative()); }

        property int VertexCount { int get() { return static_cast<int>(_mesh.vertices.size()); } }
        property int IndexCount { int get() { return static_cast<int>(_mesh.indices.size()); } }

        array<Vec3>^ GetVertices() {
            array<Vec3>^ arr = gcnew array<Vec3>(static_cast<int>(_mesh.vertices.size()));
            for (size_t i = 0; i < _mesh.vertices.size(); i++)
                arr[i] = Vec3(_mesh.vertices[i].position);
            return arr;
        }

        array<uint32_t>^ GetIndices() {
            array<uint32_t>^ arr = gcnew array<uint32_t>(static_cast<int>(_mesh.indices.size()));
            for (size_t i = 0; i < _mesh.indices.size(); i++)
                arr[i] = _mesh.indices[i];
            return arr;
        }

        void GenerateMesh(int resolutionU, int resolutionV) { _mesh = _surface->generateMesh(resolutionU, resolutionV); }
        void GenerateControlNet() { _mesh = _surface->generateControlNet(); }

        BezierSurface^ ElevateDegreeU() { return gcnew BezierSurface(_surface->elevateDegreeU()); }
        BezierSurface^ ElevateDegreeV() { return gcnew BezierSurface(_surface->elevateDegreeV()); }

    private:
        cagd::BezierSurface* _surface;
        cagd::Mesh _mesh;
    };

} // namespace cagdcli

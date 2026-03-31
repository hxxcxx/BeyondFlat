# 贝塞尔曲面（Bezier Surface）

## 数学定义

张量积贝塞尔曲面由两个方向的 Bernstein 多项式构成。对于 (m+1) × (n+1) 控制点网格 $P_{ij}$：

$$S(u,v) = \sum_{i=0}^{m} \sum_{j=0}^{n} B_i^m(u) \cdot B_j^n(v) \cdot P_{ij}, \quad u,v \in [0, 1]$$

其中 $B_i^m(u)$ 和 $B_j^n(v)$ 是 Bernstein 基多项式。

控制点网格排列：`control_points_[row][col]`
- row 方向：v 方向，共 (m+1) 行
- col 方向：u 方向，共 (n+1) 列

## 类接口

```cpp
class BezierSurface {
public:
    explicit BezierSurface(const SurfacePointGrid3d& controlPoints);
    
    // 求值
    Point3d evaluate(double u, double v) const;
    Point3d deCasteljau(double u, double v) const;  // 数值稳定
    
    // 偏导数
    std::pair<Vector3d, Vector3d> derivatives(double u, double v) const;
    
    // 法向量（归一化的叉积）
    Vector3d normal(double u, double v) const;
    
    // 阶数
    int degreeU() const { return numCols() - 1; }
    int degreeV() const { return numRows() - 1; }
    
    // 控制点数量
    int numRows() const;
    int numCols() const;
    
    // 控制点访问
    const SurfacePointGrid3d& controlPoints() const;
    void setControlPoints(const SurfacePointGrid3d& points);
    Point3d controlPoint(int row, int col) const;
    void setControlPoint(int row, int col, const Point3d& point);
    
    // 网格生成
    Mesh generateMesh(int resolutionU = 32, int resolutionV = 32) const;
    Mesh generateControlNet() const;
    
    // 升阶
    BezierSurface elevateDegreeU() const;
    BezierSurface elevateDegreeV() const;
};
```

## 核心算法

### de Casteljau 曲面求值

分两步进行：

**第一步**：对 v 方向每一行应用 de Casteljau，得到 u 方向的中间点
```cpp
// 对第 j 行控制点沿 u 方向求值
Point3d intermediate_j = deCasteljauRow(control_points_[j], u);
```

**第二步**：对所有中间点在 v 方向应用 de Casteljau
```cpp
// 对所有 intermediate_j 沿 v 方向求值
Point3d result = deCasteljauColumn(intermediates, v);
```

### 偏导数与法向量

$$\frac{\partial S}{\partial u}(u,v) = m \sum_{j=0}^{n} B_j^n(v) \cdot (P_{i+1,j} - P_{i,j})$$

$$\frac{\partial S}{\partial v}(u,v) = n \sum_{i=0}^{m} B_i^m(u) \cdot (P_{i,j+1} - P_{i,j})$$

法向量：$\vec{n}(u,v) = \frac{\partial S}{\partial u} \times \frac{\partial S}{\partial v}$

## 使用示例

```cpp
#include "src/bezier/bezier_surface.h"

using namespace cagd;

// 创建 3x3 控制点网格（二次曲面）
SurfacePointGrid3d controlPoints(3, PointVector3d(3));
controlPoints[0] = { {-1, -1, 0}, {0, -1, 1}, {1, -1, 0} };
controlPoints[1] = { {-1, 0, 1}, {0, 0, 0}, {1, 0, 1} };
controlPoints[2] = { {-1, 1, 0}, {0, 1, 1}, {1, 1, 0} };

BezierSurface surface(controlPoints);

// 在 (u=0.5, v=0.5) 处求值
Point3d p = surface.evaluate(0.5, 0.5);

// 计算法向量
Vector3d n = surface.normal(0.5, 0.5);

// 生成渲染网格
Mesh mesh = surface.generateMesh(32, 32);

// 生成控制网线框
Mesh wireframe = surface.generateControlNet();
```

## 控制点网格坐标系

```
        u →
      +---+---+---+
    + P00  P01  P02 |
v   | P10  P11  P12 |  ← 行 0, 1, 2 (v 方向)
    + P20  P21  P22 |
      +---+---+---+
```

- 行索引 (row)：v 方向，m+1 行
- 列索引 (col)：u 方向，n+1 列

## 文件位置

- 头文件：[src/bezier/bezier_surface.h](../src/bezier/bezier_surface.h)
- 实现：[src/bezier/bezier_surface.cc](../src/bezier/bezier_surface.cc)

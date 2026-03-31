# 贝塞尔曲线（Bezier Curve）

## 数学定义

贝塞尔曲线是由控制点定义的参数多项式曲线。对于给定的 n+1 个控制点 $P_0, P_1, ..., P_n$，曲线上一点定义为：

$$B(t) = \sum_{i=0}^{n} B_{i,n}(t) \cdot P_i, \quad t \in [0, 1]$$

其中 $B_{i,n}(t)$ 是 Bernstein 基多项式：

$$B_{i,n}(t) = \binom{n}{i} (1-t)^{n-i} t^i$$

## 类接口

### BezierCurve2d

```cpp
class BezierCurve2d {
public:
    explicit BezierCurve2d(const PointVector2d& controlPoints);
    
    Point2d evaluate(double t) const;              // 标准求值
    Point2d deCasteljau(double t) const;           // de Casteljau 算法（数值稳定）
    Point2d derivative(double t, int order = 1) const;  // 求导
    
    int degree() const;                            // 曲线阶数 = 控制点数 - 1
    const PointVector2d& controlPoints() const;
    void setControlPoints(const PointVector2d& points);
    
    std::pair<BezierCurve2d, BezierCurve2d> subdivide(double t) const;  // 分割曲线
    BezierCurve2d elevateDegree() const;           // 升阶
    std::optional<BezierCurve2d> reduceDegree() const;  // 降阶（近似）
};
```

### BezierCurve3d

接口与 2D 版本相同，使用 `PointVector3d` 和 `Point3d` 类型。

## 核心算法

### de Casteljau 算法

de Casteljau 算法是计算贝塞尔曲线的数值稳定方法，通过递归线性插值实现：

```
对于控制点 P0, P1, ..., Pn 和参数 t：
b[i][0] = P[i]                                    // 第 0 列是控制点
b[i][j] = (1-t) * b[i][j-1] + t * b[i+1][j-1]    // 递归计算
结果：b[0][n] 即为曲线上点 B(t)
```

几何意义：对相邻控制点按比例 t 插值，递归直到只剩一个点。

### 导数计算

贝塞尔曲线的导数仍是贝塞尔曲线：

$$B'(t) = n \sum_{i=0}^{n-1} B_{i,n-1}(t) \cdot (P_{i+1} - P_i)$$

## 使用示例

```cpp
#include "src/bezier/bezier_curve.h"

using namespace cagd;

// 创建二阶贝塞尔曲线（3个控制点）
PointVector2d points = {
    {0.0, 0.0},   // P0
    {0.5, 1.0},   // P1
    {1.0, 0.0}    // P2
};
BezierCurve2d curve(points);

// 在 t=0.5 处求值
Point2d p = curve.evaluate(0.5);

// 使用更稳定的 de Casteljau 算法
Point2d p_stable = curve.deCasteljau(0.5);

// 升阶到三阶
BezierCurve2d elevated = curve.elevateDegree();

// 分割曲线
auto [left, right] = curve.subdivide(0.5);
```

## 升阶与降阶

### 升阶（Degree Elevation）

精确操作，将 n 阶曲线提升到 n+1 阶：

$$P_i^* = \frac{i}{n+1} P_{i-1} + (1 - \frac{i}{n+1}) P_i, \quad i = 0, 1, ..., n+1$$

### 降阶（Degree Reduction）

近似操作，将 n 阶曲线降到 n-1 阶。使用均值法（前向/后向）近似。

## 文件位置

- 头文件：[src/bezier/bezier_curve.h](../src/bezier/bezier_curve.h)
- 实现：[src/bezier/bezier_curve.cc](../src/bezier/bezier_curve.cc)

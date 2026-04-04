# NURBS 曲线（Non-Uniform Rational B-Spline Curve）

## 数学定义

NURBS 曲线是 B 样条曲线的有理推广，通过为每个控制点引入权重，可以精确表示圆锥曲线（圆、椭圆、抛物线、双曲线等）。

$$C(t) = \frac{\sum_{i=0}^{n} N_{i,p}(t) \cdot w_i \cdot P_i}{\sum_{i=0}^{n} N_{i,p}(t) \cdot w_i}, \quad t \in [t_p, t_{n+1}]$$

其中：
- $P_i$：控制点
- $w_i$：控制点对应的权重
- $N_{i,p}(t)$：p 次 B 样条基函数
- $T = [t_0, t_1, ..., t_{n+p+1}]$：节点向量

### 齐次坐标表示

NURBS 的核心技巧是将 2D 曲线提升到 3D 齐次坐标空间：

$$P_i^w = (w_i x_i,\ w_i y_i,\ w_i)$$

在齐次空间中，NURBS 退化为普通的 B 样条曲线：

$$C^w(t) = \sum_{i=0}^{n} N_{i,p}(t) \cdot P_i^w$$

最终通过**透视除法**回到 2D：

$$C(t) = \left(\frac{C^w_x(t)}{C^w_w(t)},\ \frac{C^w_y(t)}{C^w_w(t)}\right)$$

其中 $C^w_w(t)$ 是齐次坐标的第三个分量（权重之和）。

### 3D NURBS 曲线

3D 版本类似，将控制点提升到 4D 齐次坐标：

$$P_i^w = (w_i x_i,\ w_i y_i,\ w_i z_i,\ w_i)$$

## 类接口

### NURBSCurve2d

```cpp
class NURBSCurve2d {
public:
    // 构造函数
    NURBSCurve2d();  // 空曲线
    
    // 指定次数和控制点，权重默认为 1.0（退化为普通 B 样条）
    NURBSCurve2d(int degree, const PointVector2d& controlPoints);
    
    // 指定权重，自动生成 clamped 节点向量
    NURBSCurve2d(int degree, const PointVector2d& controlPoints,
                 const std::vector<double>& weights);
    
    // 完整构造：指定节点向量
    NURBSCurve2d(int degree, const PointVector2d& controlPoints,
                 const std::vector<double>& weights,
                 const std::vector<double>& knots);
    
    // 求值与求导
    Point2d evaluate(double t) const;                    // 齐次坐标 de Boor + 透视除法
    Point2d derivative(double t, int order = 1) const;   // 数值微分
    
    // 属性访问
    int degree() const;
    int controlPointCount() const;
    int knotCount() const;
    
    const PointVector2d& controlPoints() const;
    void setControlPoints(const PointVector2d& points);
    
    const std::vector<double>& weights() const;
    void setWeights(const std::vector<double>& weights);
    
    const std::vector<double>& knots() const;
    void setKnots(const std::vector<double>& knots);
    
    // 节点操作
    PointVector2d insertKnot(double t, int multiplicity = 1);
    void refineKnotVector();                             // 节点向量加密
    std::pair<double, double> domain() const;
    bool isValid() const;
};
```

### NURBSCurve3d

接口与 2D 版本对称，使用 `PointVector3d` / `Point3d` / `Vector4d` 类型。

## 核心算法

### 齐次坐标 de Boor 算法

```
输入：参数 t，控制点 P[i]，权重 w[i]，节点向量 T，度数 p
输出：曲线上点 C(t)

1. 将控制点提升到齐次坐标：
     d[i] = (w[i] * P[i].x, w[i] * P[i].y, w[i])   // 2D → 3D

2. 找到节点区间 k 使得 t ∈ [t_k, t_{k+1})

3. 取齐次控制点 d[j] = P^w_{k-p+j}, j = 0..p

4. De Boor 递推（在齐次空间中执行标准 B 样条递推）：
     对于 r = 1..p：
       对于 j = p..r（逆序）：
         α = (t - t_{k-p+j}) / (t_{k+1+p-r} - t_{k-p+j})
         d[j] = (1-α) * d[j-1] + α * d[j]

5. 透视除法：
     C(t) = (d[p].x / d[p].z, d[p].y / d[p].z)
```

### 节点插入（Boehm 算法）

NURBS 的节点插入在**齐次空间**中进行：

```
对于新节点 t，找到节点区间 k：

1. 不受影响的控制点直接复制
2. 受影响的控制点在齐次空间中线性插值：
     α = (t - t_i) / (t_{i+p} - t_i)
     w_new = (1-α) * w[i-1] + α * w[i]
     P_new = ((1-α) * w[i-1] * P[i-1] + α * w[i] * P[i]) / w_new
3. 插入新节点到节点向量
```

节点插入不改变曲线几何形状，但增加控制点数量。

### 节点向量加密

对每个节点区间的中点执行节点插入，加密后的曲线更易交互编辑。

## 权重的几何意义

权重 $w_i$ 控制控制点 $P_i$ 对曲线的"吸引力"：

- **$w_i = 1$**：标准 B 样条行为
- **$w_i > 1$**：曲线被拉向 $P_i$（"拉"效果）
- **$w_i < 1$**：曲线远离 $P_i$（"推"效果）
- **$w_i = 0$**：该控制点完全不影响曲线（相当于移除）
- **$w_i \to \infty$**：曲线退化为经过 $P_i$ 的折线

## 圆锥曲线的 NURBS 表示

### 单位圆

使用 9 个控制点、2 次 NURBS，节点向量为：

$$T = \{0,0,0,\ \tfrac{1}{4},\tfrac{1}{4},\ \tfrac{1}{2},\tfrac{1}{2},\ \tfrac{3}{4},\tfrac{3}{4},\ 1,1,1\}$$

控制点与权重：

| 控制点 | 坐标 | 权重 |
|--------|------|------|
| $P_0$ | $(1, 0)$ | 1 |
| $P_1$ | $(1, 1)$ | $\frac{\sqrt{2}}{2}$ |
| $P_2$ | $(0, 1)$ | 1 |
| $P_3$ | $(-1, 1)$ | $\frac{\sqrt{2}}{2}$ |
| $P_4$ | $(-1, 0)$ | 1 |
| $P_5$ | $(-1, -1)$ | $\frac{\sqrt{2}}{2}$ |
| $P_6$ | $(0, -1)$ | 1 |
| $P_7$ | $(1, -1)$ | $\frac{\sqrt{2}}{2}$ |
| $P_8$ | $(1, 0)$ | 1 |

内节点重复度为 2，使得每个四分之一圆弧是独立的贝塞尔段。角点处权重为 1，肩点处权重为 $\cos(\pi/4) = \frac{\sqrt{2}}{2}$。

### 90° 圆弧

最简表示只需 3 个控制点：

| 控制点 | 坐标 | 权重 |
|--------|------|------|
| $P_0$ | $(r, 0)$ | 1 |
| $P_1$ | $(r, r)$ | $\frac{\sqrt{2}}{2}$ |
| $P_2$ | $(0, r)$ | 1 |

### 椭圆

与圆类似，但两个轴方向使用不同的半径。

## 与 B 样条的关系

NURBS 是 B 样条的推广：

| 特性 | B 样条 | NURBS |
|------|--------|-------|
| 控制点 | $P_i$ | $P_i$ + 权重 $w_i$ |
| 基函数 | 多项式 | 有理多项式 |
| 圆锥曲线 | 近似 | 精确 |
| 仿射不变性 | ✅ | ✅ |
| 透视不变性 | ❌ | ✅ |

当所有权重 $w_i = 1$ 时，NURBS 退化为普通 B 样条。

## 使用示例

### 创建圆形 NURBS

```cpp
#include "src/nurbs/nurbs_curve.h"

using namespace cagd;

int degree = 2;
double r = 1.0;
double sq2 = std::sqrt(2.0) / 2.0;

PointVector2d controlPoints = {
    {r, 0}, {r, r}, {0, r},
    {-r, r}, {-r, 0},
    {-r, -r}, {0, -r},
    {r, -r}, {r, 0}
};

std::vector<double> weights = {1, sq2, 1, sq2, 1, sq2, 1, sq2, 1};
std::vector<double> knots = {0, 0, 0, 0.25, 0.25, 0.5, 0.5, 0.75, 0.75, 1, 1, 1};

NURBSCurve2d circle(degree, controlPoints, weights, knots);

// 在圆上求值
Point2d p = circle.evaluate(0.25);  // ≈ (0, 1) - 圆的顶部
Point2d q = circle.evaluate(0.5);   // ≈ (-1, 0) - 圆的左端
```

### 简单加权曲线

```cpp
// 5 个控制点，3 次 NURBS
PointVector2d points = {{0,0}, {1,3}, {3,1}, {5,3}, {6,0}};

// 第三个控制点权重更高 → 曲线被拉向它
std::vector<double> weights = {1.0, 1.0, 3.0, 1.0, 1.0};

NURBSCurve2d curve(3, points, weights);
// clamped 节点向量自动生成

// 求值与求导
Point2d p = curve.evaluate(0.5);
Point2d tangent = curve.derivative(0.5);
```

### 节点操作

```cpp
// 插入节点
curve.insertKnot(0.4, 1);  // 在 t=0.4 处插入一个节点

// 加密节点向量
curve.refineKnotVector();

// 查看信息
auto [tmin, tmax] = curve.domain();
int nPoints = curve.controlPointCount();
```

### 3D NURBS

```cpp
#include "src/nurbs/nurbs_curve.h"

// 3D 控制点
PointVector3d points3d = {{0,0,0}, {1,2,1}, {3,1,2}, {5,3,1}, {6,0,0}};
std::vector<double> weights = {1, 1, 1, 1, 1};

NURBSCurve3d curve3d(3, points3d, weights);
Point3d p3d = curve3d.evaluate(0.5);
```

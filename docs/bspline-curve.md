# B 样条曲线（B-Spline Curve）

## 数学定义

B 样条曲线由控制点和节点向量（knot vector）定义：

$$C(t) = \sum_{i=0}^{n} N_{i,p}(t) \cdot P_i, \quad t \in [t_p, t_{n+1}]$$

其中：
- $P_i$：控制点
- $N_{i,p}(t)$：p 次 B 样条基函数
- $T = [t_0, t_1, ..., t_{n+p+1}]$：节点向量

## 类接口

### BSplineCurve2d

```cpp
class BSplineCurve2d {
public:
    // 默认构造（空曲线）
    BSplineCurve2d();
    
    // 自动生成均匀节点向量
    BSplineCurve2d(int degree, const PointVector2d& controlPoints);
    
    // 显式指定节点向量
    BSplineCurve2d(int degree, const PointVector2d& controlPoints, 
                   const std::vector<double>& knots);
    
    // 求值与求导
    Point2d evaluate(double t) const;                    // de Boor 算法
    Point2d derivative(double t, int order = 1) const;
    
    // 属性访问
    int degree() const { return degree_; }
    int controlPointCount() const { return control_points_.size(); }
    int knotCount() const { return knots_.size(); }
    
    const PointVector2d& controlPoints() const;
    void setControlPoints(const PointVector2d& points);
    
    const std::vector<double>& knots() const;
    void setKnots(const std::vector<double>& knots);
    
    // 节点操作
    PointVector2d insertKnot(double t, int multiplicity = 1);
    std::pair<double, double> domain() const;  // 有效参数范围
    bool isValid() const;
};
```

## 核心算法

### Cox-de Boor 递归公式

B 样条基函数定义：

$$N_{i,0}(t) = \begin{cases} 1 & t_i \leq t < t_{i+1} \\ 0 & \text{otherwise} \end{cases}$$

$$N_{i,p}(t) = \frac{t - t_i}{t_{i+p} - t_i} N_{i,p-1}(t) + \frac{t_{i+p+1} - t}{t_{i+p+1} - t_{i+1}} N_{i+1,p-1}(t)$$

### de Boor 算法

计算曲线上点的有效算法：

```
输入：参数 t，控制点 P[i](i=0..n)，节点向量 T，度数 p
输出：曲线上点 C(t)

1. 找到 k 使得 t ∈ [t_k, t_{k+1})
2. 初始化 d[i] = P_{i+k-p}, i = 0..p
3. 对于 j = 1..p：
     对于 i = 0..p-j：
       α = (t - t_{i+k-j}) / (t_{i+p+1} - t_{i+k-j})
       d[i] = (1-α) * d[i] + α * d[i+1]
4. 返回 d[0]
```

## 节点向量

### 均匀节点向量

自动生成的均匀节点向量：

$$T = [0, 0, ..., 0, \frac{1}{m}, \frac{2}{m}, ..., \frac{m-1}{m}, 1, 1, ..., 1]$$

首尾各有 p+1 个重复节点（clamped 条件）。

### 节点插入（Knot Insertion）

在参数 t 处插入新节点，保持曲线形状不变：

$$P^* = \text{insertKnot}(t, \text{multiplicity})$$

## 使用示例

```cpp
#include "src/bspline/bspline_curve.h"

using namespace cagd;

// 创建三次均匀 B 样条曲线（4个控制点）
PointVector2d points = {
    {0.0, 0.0},
    {0.0, 1.0},
    {1.0, 1.0},
    {1.0, 0.0}
};
BSplineCurve2d curve(3, points);  // degree = 3

// 在 t=0.5 处求值
Point2d p = curve.evaluate(0.5);

// 获取节点向量
std::vector<double> knots = curve.knots();

// 获取有效参数范围
auto [tmin, tmax] = curve.domain();

// 插入新节点
PointVector2d newControlPoints = curve.insertKnot(0.3, 1);

// 创建带显式节点向量的曲线
std::vector<double> customKnots = {0, 0, 0, 0, 0.25, 0.5, 0.75, 1, 1, 1, 1};
BSplineCurve2d curve2(3, points, customKnots);
```

## 贝塞尔与 B 样条的关系

贝塞尔曲线是 B 样条的特殊情况：
- 当节点向量首尾各有 n+1 个重复节点时
- B 样条退化为贝塞尔曲线

## 文件位置

- 头文件：[src/bspline/bspline_curve.h](../src/bspline/bspline_curve.h)
- 实现：[src/bspline/bspline_curve.cc](../src/bspline/bspline_curve.cc)

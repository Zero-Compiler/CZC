# Zero 宏系统示例：几何图形库

## 概述

本示例展示了 Zero 的三种宏系统如何协同工作，实现一个完整的几何图形库。通过这个例子，你可以理解：

- **声明式宏** (`!`)：用于代码模板替换，简化重复代码
- **过程宏** (`#`)：用于自动为类型生成 trait 实现
- **编译期宏** (`@`)：用于编译期类型计算和约束

***

## 代码

```rust
/* ============================================
   示例：几何图形库
   ============================================ */

// ========== 1. 声明式宏：简化重复代码 ==========
macro !vec2(x, y) {
    Vector2 { x: x, y: y }
}

macro !area_formula(shape, formula) {
    fn area(self) -> Float {
        formula
    }
}

// ========== 2. 过程宏：自动实现 trait ==========
macro #derive_debug(T: Type) {
    impl Debug for T {
        fn fmt(self, f: Formatter) -> Result {
            let type_name = stringify!(T);
            let fields = @get_fields(T);  // 编译期求值展开
            /* 例如这里对Vector2展开后相当于：
               let fields = [
                   { name: "x", value: self.x },
                   { name: "y", value: self.y }
               ];
               这部分由编译期宏@get_fields实现
            */
            !write(f, "{} {{ ", type_name)?;
            for field in fields {
                !write(f, "{}: {:?}, ", field.name, field.value)?;
            }
            !write(f, "}}")
        }
    }
}

macro #derive_clone(T: Type) {
    impl Clone for T {
        fn clone(self) -> T {
            let fields = @get_fields(T);
            T {
                // 遍历所有字段并克隆
                ..fields.map(|f| f.clone())
            }
        }
    }
}

// ========== 3. 编译期宏：编译期求值解决 ==========
macro @ensure_numeric(T: Type) -> Boolean {
    T == Integer || T == Float
}

macro @max_size_type(T1: Type, T2: Type) -> Type {
    if sizeof(T1) >= sizeof(T2) {
        T1
    } else {
        T2
    }
}

// ========== 类型定义 ==========
type Vector2 = struct {
    x: Float,
    y: Float
}

type Circle = struct {
    center: Vector2,
    radius: Float
}

type Rectangle = struct {
    top_left: Vector2,
    width: Float,
    height: Float
}

type Triangle = struct {
    p1: Vector2,
    p2: Vector2,
    p3: Vector2
}

// ========== 使用过程宏自动实现 trait ==========
// 方式1：函数式调用
#derive_debug(Circle);
#derive_debug(Rectangle);
#derive_debug(Triangle);

// 方式2：方法式调用（可选语法）
Vector2.#derive_debug();
Vector2.#derive_clone();

// 批量 derive
Circle.#derive_debug().#derive_clone();

// ========== 为图形实现面积计算 ==========
impl Circle {
    !area_formula(Circle, 3.14159 * self.radius * self.radius)
}

impl Rectangle {
    !area_formula(Rectangle, self.width * self.height)
}

impl Triangle {
    fn area(self) -> Float {
        // 使用海伦公式
        let a = distance(self.p1, self.p2);
        let b = distance(self.p2, self.p3);
        let c = distance(self.p3, self.p1);
        let s = (a + b + c) / 2.0;
        return sqrt(s * (s - a) * (s - b) * (s - c));
    }
}

// ========== 泛型函数使用编译期宏 ==========
fn max<T>(a: T, b: T) -> T 
    where @ensure_numeric(T)  // 编译期类型约束
{
    if a > b { a } else { b }
}

fn add_with_promotion<T1, T2>(a: T1, b: T2) -> @max_size_type(T1, T2) {
    // 自动提升到更大的类型
    return (a as @max_size_type(T1, T2)) + (b as @max_size_type(T1, T2));
}

// ========== 辅助函数 ==========
fn distance(p1: Vector2, p2: Vector2) -> Float {
    let dx = p2.x - p1.x;
    let dy = p2.y - p1.y;
    return sqrt(dx * dx + dy * dy);
}

// ========== 主程序 ==========
pub fn main() -> Integer {
    // 使用声明式宏创建向量
    let center = !vec2(0.0, 0.0);
    let p1 = !vec2(0.0, 0.0);
    let p2 = !vec2(4.0, 0.0);
    let p3 = !vec2(2.0, 3.0);
    
    // 创建图形
    let circle = Circle { 
        center: center, 
        radius: 5.0 
    };
    
    let rect = Rectangle { 
        top_left: !vec2(0.0, 0.0), 
        width: 4.0, 
        height: 3.0 
    };
    
    let triangle = Triangle { 
        p1: p1, 
        p2: p2, 
        p3: p3 
    };
    
    // 使用自动生成的 Debug 实现打印
    print("Circle: {:?}", circle);
    // 输出: Circle { center: Vector2 { x: 0.0, y: 0.0 }, radius: 5.0 }
    
    print("Rectangle: {:?}", rect);
    // 输出: Rectangle { top_left: Vector2 { x: 0.0, y: 0.0 }, width: 4.0, height: 3.0 }
    
    // 计算面积
    print("Circle area: {}", circle.area());        // 78.54
    print("Rectangle area: {}", rect.area());       // 12.0
    print("Triangle area: {}", triangle.area());    // 6.0
    
    // 使用编译期类型约束的泛型函数
    let max_int = max(10, 20);           // T = Integer
    let max_float = max(3.14, 2.71);     // T = Float
    
    // 编译期类型提升
    let result = add_with_promotion(100, 3.14);
    // 返回类型自动推导为 Float
    print("Result: {}", result);  // 103.14
    
    // 使用 Clone
    let circle2 = circle.clone();
    print("Cloned circle: {:?}", circle2);
    
    return 0;
}
```

***

## 三种宏的详细说明

### 1. 声明式宏 (`!`) - 代码模板替换

**定义**：

```rust
macro !vec2(x, y) {
    Vector2 { x: x, y: y }
}
```

**用途**：简化重复的代码模式，类似 C 语言的 `#define`，但类型安全。

**使用示例**：

```rust
// 不用宏：重复写构造函数
let p1 = Vector2 { x: 0.0, y: 0.0 };
let p2 = Vector2 { x: 4.0, y: 0.0 };

// 用宏：简洁清晰
let p1 = !vec2(0.0, 0.0);
let p2 = !vec2(4.0, 0.0);
```

**展开过程**：

- **调用**：`!vec2(0.0, 0.0)`
- **展开**：`Vector2 { x: 0.0, y: 0.0 }`

***

### 2. 过程宏 (`#`) - 自动实现 Trait

**定义**：

```rust
macro #derive_debug(T: Type) {
    impl Debug for T {
        fn fmt(self, f: Formatter) -> Result {
            let type_name = stringify!(T);
            let fields = @get_fields(T);  // 编译期求值
            !write(f, "{} {{ ", type_name)?;
            for field in fields {
                !write(f, "{}: {:?}, ", field.name, field.value)?;
            }
            !write(f, "}}")
        }
    }
}
```

**用途**：读取类型结构，自动生成 trait 实现代码。

**使用示例**：

```rust
// 函数式调用
#derive_debug(Circle);

// 方法链式调用
Circle.#derive_debug().#derive_clone();
```

**生成的代码**（以 `Circle` 为例）：

```rust
impl Debug for Circle {
    fn fmt(self, f: Formatter) -> Result {
        !write(f, "Circle {{ ")?;
        !write(f, "center: {:?}, ", self.center)?;
        !write(f, "radius: {:?}, ", self.radius)?;
        !write(f, "}}")
    }
}
```

**关键点**：

- `@get_fields(T)` 是编译期宏，在编译时读取类型字段信息
- 对 `Circle` 展开后得到 `[{ name: "center", ... }, { name: "radius", ... }]`
- 循环遍历字段，自动生成打印代码

***

### 3. 编译期宏 (`@`) - 类型级计算

**定义**：

```rust
macro @ensure_numeric(T: Type) -> Boolean {
    T == Integer || T == Float || T == Double
}

macro @max_size_type(T1: Type, T2: Type) -> Type {
    if sizeof(T1) >= sizeof(T2) {
        T1
    } else {
        T2
    }
}
```

**用途**：在编译时进行类型检查、类型推导、类型转换等操作。

**使用示例 1：类型约束**

```rust
fn max<T>(a: T, b: T) -> T 
    where @ensure_numeric(T)  // 编译时检查 T 是否为数字类型
{
    if a > b { a } else { b }
}

// 合法：Integer 是数字类型
let x = max(10, 20);

// 编译错误：String 不是数字类型
let y = max("a", "b");  // 编译器报错
```

**使用示例 2：类型自动提升**

```rust
fn add_with_promotion<T1, T2>(a: T1, b: T2) -> @max_size_type(T1, T2) {
    return (a as @max_size_type(T1, T2)) + (b as @max_size_type(T1, T2));
}

// Integer (4字节) + Float (4字节) → Float
let result1 = add_with_promotion(100, 3.14);  // 返回类型: Float

// Integer (4字节) + Double (8字节) → Double
let result2 = add_with_promotion(100, 3.14159265359);  // 返回类型: Double
```

**展开过程**：

- **调用**：`@max_size_type(Integer, Float)`
- **编译时计算**：`sizeof(Integer) = 4, sizeof(Float) = 4 → 返回 Float`
- **类型签名变为**：`fn add_with_promotion(a: Integer, b: Float) -> Float`

***

## 输出示例

运行 `main()` 函数的输出：

```
Circle: Circle { center: Vector2 { x: 0.0, y: 0.0 }, radius: 5.0 }
Rectangle: Rectangle { top_left: Vector2 { x: 0.0, y: 0.0 }, width: 4.0, height: 3.0 }
Circle area: 78.53975
Rectangle area: 12.0
Triangle area: 6.0
Result: 103.14
Cloned circle: Circle { center: Vector2 { x: 0.0, y: 0.0 }, radius: 5.0 }
```

***

## 设计亮点

### 1. 清晰的语法区分

- `!`：声明式宏（代码替换）
- `#`：过程宏（trait 生成）
- `@`：编译期宏（类型计算）

### 2. 灵活的调用方式

```rust
// 函数式
#derive_debug(Circle);

// 方法链式
Circle.#derive_debug().#derive_clone();
```

### 3. 编译期和运行时的清晰分离

- `@get_fields(T)`：编译时执行，返回类型信息
- `circle.area()`：运行时执行，计算具体值

### 4. 强大的类型系统集成

- 编译期类型约束：`where @ensure_numeric(T)`
- 编译期类型推导：`-> @max_size_type(T1, T2)`

***

## 与其他语言对比

| 特性           | Zero                      | Rust                         | Zig                    |
|----------------|-------------------------------|------------------------------|------------------------|
| 代码模板       | `!vec2(x, y)`                 | `macro_rules!`               | 内联函数               |
| Trait 生成     | `#derive_debug(T)`            | `#[derive(Debug)]`           | `comptime` 函数        |
| 编译期计算     | `@ensure_numeric(T)`          | `const fn`（受限）           | `comptime` 表达式      |
| 语法统一性     | 三种前缀清晰区分            | 属性/宏混杂                | `comptime` 统一      |
| 易用性         | 函数式调用直观              | 属性语法冗长               | 与普通代码一致       |

# AST (Abstract Syntax Tree) Framework

## 概述

AST 模块是 Zero 语言编译器的语义层表示，从 CST（具体语法树）转换而来。AST 移除了冗余的语法信息，保留了程序的语义结构，为后续的类型检查、代码优化和代码生成提供基础。

## 目录结构

```
czc/ast/              # AST 头文件
  ├── ast_node.hpp    # AST 节点类型定义
  ├── ast_visitor.hpp # 访问者模式接口
  └── ast_builder.hpp # CST 到 AST 的转换器

src/ast/              # AST 实现文件
  ├── ast_builder.cpp # 转换器实现
  └── ast_visitor.cpp # 访问者实现
```

## 核心组件

### 1. AST 节点层次结构

```
ASTNode (基类)
├── Expression (表达式基类)
│   ├── IntegerLiteral
│   ├── FloatLiteral
│   ├── StringLiteral
│   ├── BooleanLiteral
│   ├── Identifier
│   ├── BinaryOpExpr
│   ├── UnaryOpExpr
│   └── ...
├── Statement (语句基类)
│   ├── BlockStmt
│   ├── ExprStmt
│   ├── ReturnStmt
│   ├── IfStmt
│   └── ...
├── Declaration (声明基类)
│   ├── VarDecl
│   ├── FunctionDecl
│   ├── StructDecl
│   └── ...
└── Type (类型基类)
    ├── PrimitiveType
    ├── ArrayType
    ├── TupleType
    └── ...
```

### 2. ASTBuilder - CST 到 AST 转换器

`ASTBuilder` 负责将 CST 转换为 AST，主要功能包括：

- **语法简化**：移除括号、分隔符等语法噪音
- **语义提取**：提取变量名、类型、值等语义信息
- **结构重组**：建立更适合语义分析的树结构
- **位置保留**：保留源码位置信息用于错误报告

**使用示例：**

```cpp
#include "czc/ast/ast_builder.hpp"
#include "czc/parser/parser.hpp"
#include "czc/lexer/lexer.hpp"

// 1. 词法分析
Lexer lexer(source_code, "example.zero");
auto tokens = lexer.tokenize();

// 2. 语法分析 -> CST
Parser parser(tokens);
auto cst = parser.parse();

// 3. CST -> AST
ASTBuilder builder;
auto ast = builder.build(cst.get());

// 4. 使用 AST
// ast->get_declarations() ...
```

### 3. ASTVisitor - 访问者模式

访问者模式用于遍历和处理 AST，支持以下操作：

- **类型检查**：验证类型一致性
- **代码生成**：生成目标代码
- **代码优化**：常量折叠、死代码消除等
- **静态分析**：查找潜在问题
- **Pretty Printing**：格式化输出 AST 结构

**实现自定义访问者：**

```cpp
#include "czc/ast/ast_visitor.hpp"

class MyVisitor : public ASTBaseVisitor {
public:
  void visit_integer_literal(IntegerLiteral* node) override {
    std::cout << "Found integer: " << node->get_value() << std::endl;
  }

  void visit_identifier(Identifier* node) override {
    std::cout << "Found identifier: " << node->get_name() << std::endl;
  }
};

// 使用
MyVisitor visitor;
ast->accept(&visitor);
```

## AST vs CST 的区别

| 特性 | CST | AST |
|------|-----|-----|
| **完整性** | 包含所有语法细节（括号、分号、空格） | 只保留语义信息 |
| **节点数量** | 节点多，树深 | 节点少，树浅 |
| **用途** | 代码格式化、语法高亮 | 类型检查、代码生成、优化 |
| **表示** | `let x: i32 = (1 + 2);` 所有细节 | `VarDecl(x, i32, BinaryOp(+, 1, 2))` |

**CST 示例：**

```
Program
  └── VarDeclaration
      ├── Keyword "let"
      ├── Identifier "x"
      ├── Colon ":"
      ├── TypeAnnotation "i32"
      ├── Equals "="
      ├── ParenExpr "("
      │   └── BinaryExpr
      │       ├── IntegerLiteral "1"
      │       ├── Plus "+"
      │       └── IntegerLiteral "2"
      ├── CloseParen ")"
      └── Semicolon ";"
```

**AST 示例：**

```
Program
  └── VarDecl
      ├── name: "x"
      ├── type: PrimitiveType(i32)
      └── init: BinaryOp(Add)
          ├── left: IntegerLiteral(1)
          └── right: IntegerLiteral(2)
```

## 当前实现状态

### 已实现

1. **基础框架**
   - AST 节点类型定义（`ASTNodeKind` 枚举）
   - 基础节点类（`ASTNode`, `Expression`, `Statement`, `Declaration`, `Type`）
   - 核心节点实现（`Program`, `Identifier`, `IntegerLiteral`, `BinaryOpExpr`, `BlockStmt`）

2. **访问者模式**
   - `ASTVisitor` 接口定义
   - `ASTBaseVisitor` 基类（提供默认实现）
   - `ASTPrinter` 调试工具（骨架）

3. **ASTBuilder 骨架**
   - CST -> AST 转换框架
   - 运算符解析（二元、一元）
   - 字面量解析（整数、浮点数、字符串）
   - 节点分发机制

4. **测试**
   - 基础 AST 节点创建测试
   - ASTBuilder 框架测试
   - 8 个测试用例全部通过

### 待实现

1. **完整的节点转换**
   - 变量声明 (`VarDecl`)
   - 函数声明 (`FunctionDecl`)
   - 结构体声明 (`StructDecl`)
   - 所有语句类型（`IfStmt`, `WhileStmt`, `ReturnStmt` 等）
   - 所有表达式类型（调用、索引、成员访问等）
   - 所有类型节点（数组、元组、函数类型等）

2. **类型检查器**
   - 类型推导
   - 类型兼容性检查
   - 类型错误报告

3. **语义分析**
   - 作用域分析
   - 名称解析
   - 控制流分析

4. **代码生成**
   - LLVM IR 生成
   - 或其他目标代码生成

## 使用指南

### 添加新的 AST 节点类型

1. 在 `ast_node.hpp` 中添加枚举值：

```cpp
enum class ASTNodeKind {
  // ...
  NewNodeType,  // 新节点类型
};
```

2. 定义节点类：

```cpp
class NewNode : public Expression {  // 或 Statement/Declaration
public:
  NewNode(/* params */, const utils::SourceLocation& location)
      : Expression(ASTNodeKind::NewNodeType, location) {}
  
  // 添加访问器方法
private:
  // 添加成员变量
};
```

3. 在 `ASTBuilder` 中添加转换方法：

```cpp
std::shared_ptr<Expression> build_new_node(const cst::CSTNode* cst_node);
```

4. 在 `ASTVisitor` 中添加访问方法：

```cpp
virtual void visit_new_node(NewNode* node) = 0;
```

### 实现转换逻辑

在 `ast_builder.cpp` 中实现转换：

```cpp
std::shared_ptr<Expression> ASTBuilder::build_new_node(
    const cst::CSTNode* cst_node) {
  // 1. 提取子节点
  // 2. 递归转换子节点
  // 3. 创建 AST 节点
  // 4. 返回结果
}
```

## 设计原则

1. **不可变性**：AST 节点创建后不应修改（除了类型信息）
2. **位置保留**：所有节点保留源码位置信息
3. **类型安全**：使用 `std::shared_ptr` 管理节点生命周期
4. **访问者模式**：使用访问者模式遍历 AST，避免在节点类中添加过多操作
5. **语义清晰**：AST 应该清晰地表达程序的语义，而不是语法

## 测试

运行 AST 测试：

```bash
# 编译
make build

# 运行所有测试
make test

# 只运行 AST 测试
./build/tests/test_ast

# 查看详细输出
./build/tests/test_ast --gtest_filter=ASTTest.*
```

## 下一步计划

1. **实现完整的节点转换**
   - 优先实现常用节点（变量声明、函数声明、基本表达式）
   - 逐步添加其他节点类型

2. **完善访问者实现**
   - 实现 `ASTPrinter` 的完整功能
   - 添加更多实用的访问者（如类型检查器）

3. **增强测试覆盖**
   - 为每种节点类型添加测试
   - 测试复杂的嵌套结构
   - 测试错误情况

4. **性能优化**
   - 使用对象池管理节点
   - 优化内存布局
   - 减少不必要的拷贝

## 贡献指南

欢迎贡献！在实现新功能时：

1. 遵循现有的代码风格
2. 添加完整的文档注释
3. 编写单元测试
4. 更新本 README

### 基本注释规范

- 单行注释：`// 这是单行注释`
- 多行注释：`/* 这是多行注释 */`，常用于复杂说明或段落性注释。[8]

### 文件头注释（推荐Doxygen格式）

```cpp
/**
 * @file 文件名.cpp
 * @brief 文件功能简述
 * @author 作者名(BegoniaHe)
 */
```

- 头部信息包含作者、时间、版本、简要描述等，有助于文档整理和追溯。

### 函数注释模板

```cpp
/**
 * @brief 函数功能简述
 * @details 补充详细说明（可选）
 * @param 参数名 参数作用描述
 * @param 参数名 参数作用描述
 * @return 返回值说明
 * @retval 具体返回值说明（可选）
 * @note 备注（可选）
 * @attention 注意事项（可选）
 * @warning 警告说明（可选）
 * @exception 异常情况（可选）
 */
```

- `@brief`用于简述功能；`@param/@return/@retval`分别描述参数和返回值；`@note/@attention`可用于补充说明和警告。

给所有注释规范化,统一用中文写

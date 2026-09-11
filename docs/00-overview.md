# 计算器架构

应用采用单页 MVVM 结构，并遵守 LILYGO UI AppKit 的生命周期约束。

```text
触控事件
  -> CalculatorViewModel::perform()
  -> CalculatorModel
  -> LVGL Subject
  -> 已绑定的公式与结果标签
```

## Model

`CalculatorModel` 是纯 C++17 代码，不依赖 LVGL。它负责：

- 表达式组装与输入边界处理；
- 带优先级的递归下降解析；
- 科学函数、角度模式和错误域检查；
- 公式、结果、内存和二级功能键状态；
- 最多 12 位有效数字的显示格式化。

`evaluate_expression()` 会在求值前补齐未闭合的右括号，但不会掩盖其他语法错误。除零、偶次负数开方、无效对数和溢出均返回失败结果。

## ViewModel

`CalculatorViewModel` 将按钮命令映射为 Model 操作，并通过字符串 Subject 发布：

- 主结果；
- 当前表达式；
- `AC` / `C` 状态；
- `Rad` / `Deg` 状态。

ViewModel 的生命周期由 `app.cpp` 管理，并始终长于 View 及其 Subject 绑定。

## View

`CalculatorView` 只创建 LVGL 对象、响应容器尺寸变化并转发按钮事件。页面使用 Flex 分配显示区和键盘区，两个键盘内部使用 Grid；方向变化只更新布局，不重建页面，因此不会丢失输入或内存状态。

关键对象均设置 `calculator_*` 稳定名称，供无头渲染测试定位。

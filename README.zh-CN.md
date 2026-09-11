# LILYGO UI Calculator

[English](README.md) | **简体中文**

LILYGO UI Calculator 是面向 LILYGO UI 的触控科学计算器，基于 C++17、[LILYGO UI AppKit](https://github.com/LILYGO-UI/cm0-appkit) 和 LVGL 9.5 构建。竖屏提供接近 iPhone 计算器的四列基础键盘，横屏自动展开科学计算键区。

项目提供 CM0 ARM64 Linux 设备构建，以及用于开发和测试的 SDL2 桌面模拟器。

## 预览

<p><img src="assets/previews/preview1.png" alt="竖屏计算器界面" width="240"></p>

## 功能

- **基础计算：** 四则运算、括号、正负号、百分比、小数和科学计数法。
- **表达式显示：** 实时表达式预览和标准运算符优先级。
- **科学函数：** 幂、根、三角、反三角、双曲、对数、阶乘和绝对值函数。
- **常量与内存：** `e`、`pi`、随机数、弧度/角度模式，以及 `mc`、`m+`、`m-`、`mr` 操作。
- **错误处理：** 自动补齐未闭合括号；除零、无效定义域和溢出显示 `Error`。
- **响应式布局：** 支持竖屏 `568x1232` 和横屏 `1232x568`，旋转时保留表达式、模式和内存状态。

界面和状态层使用 LVGL；计算 Model 不依赖 LVGL，可独立测试。

## 开发环境搭建

项目标识、版本和软件包元数据由 [`lpm.toml`](lpm.toml) 提供。[`CMakePresets.json`](CMakePresets.json) 定义 `host-simulator` 和 `cm0-cross` 配置。

### 获取源码

```sh
git clone --recurse-submodules https://github.com/LILYGO-UI/lilygo-ui-calculator.git
cd lilygo-ui-calculator
git submodule update --init --recursive
```

### 主机依赖与 LPM

需要 CMake 3.21+、Python 3.11+、C++17 编译器、`pkg-config`、`patch`、SDL2 和 FreeType。安装 LPM：

```sh
sudo apt install build-essential cmake git pkg-config patch libsdl2-dev libfreetype6-dev python3 python3-venv python3-pip
python3 -m venv .venv
. .venv/bin/activate
python -m pip install "git+https://github.com/LILYGO-UI/lpm.git"
```

设备构建还需要 AArch64 交叉编译器：`sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu`。SDK 要求详见 [CM0BspBuilder](https://github.com/LILYGO-UI/CM0BspBuilder)。

## 构建与测试

```sh
lpm start --foreground
lpm test
lpm pack
```

`lpm start` 启动 SDL2 模拟器，`lpm test` 运行核心、ViewModel 以及横竖屏渲染测试，`lpm pack` 生成输出到 `dist/` 的 ARM64 `.deb`。渲染结果位于 `build/host-simulator`，修改 UI 后请检查生成的 PPM 图片。

底层 CMake 命令及设备构建细节见英文 README；界面验收要求见 [UI 开发规范](docs/01-user-interface.md)。

## 文档

| 指南 | 内容 |
| --- | --- |
| [架构概览](docs/00-overview.md) | Model、ViewModel、View 边界和数据流 |
| [UI 开发规范](docs/01-user-interface.md) | 布局、字体、颜色、交互状态和视觉检查 |

## 致谢

计算功能参考 [CardputerZero/Calculator](https://github.com/CardputerZero/Calculator)，UI 集成基于 [LILYGO UI AppKit](https://github.com/LILYGO-UI/cm0-appkit) 和 [LVGL](https://lvgl.io/) 重新实现。

## License

MIT

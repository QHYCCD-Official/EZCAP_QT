# EZCAP_QT

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Qt](https://img.shields.io/badge/Qt-5.12.12-brightgreen.svg)](https://www.qt.io/)
[![OpenCV](https://img.shields.io/badge/OpenCV-4.x-blue.svg)](https://opencv.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)]()

**EZCAP_QT** 是一款基于 Qt 框架开发的 **QHYCCD 天文相机采集控制软件**，提供实时图像采集、显示、处理、温控管理、暗场校准、FITS 格式导出等完整的天文摄影功能。

---

## 📸 功能特性

### 核心功能
- **实时图像采集** — 支持 QHYCCD 系列天文相机的实时预览与采集
- **多相机支持** — 兼容 QHYCCD 全系列相机（USB3.0 / GigE / PCIe）
- **图像显示** — 实时直方图、FPS 显示、亮度/对比度调节
- **FITS 格式** — 完整的 FITS 文件读写支持，包含完整元数据

### 相机控制
- **温控管理** — 制冷相机温度控制与监控
- **曝光控制** — 支持多种曝光模式和读取模式
- **批量采集** — 自动批量拍摄（Burst Mode）
- **触发模式** — 外部触发采集

### 图像处理
- **暗场校准** — 暗场帧采集与校准
- **图像分析** — 星点检测、中心校正
- **帧校准** — 偏置帧/平场帧校准工具

### 天文集成
- **PHD2 导星联动** — 与 PHD2 导星软件集成
- **GPS 支持** — GPS 位置信息嵌入 FITS 头文件
- **CFW 滤镜轮** — 支持 QHY CFW 系列滤镜轮控制

### 扩展功能
- **MCP Server** — 内置 MCP (Model Context Protocol) 服务器，支持 IPC 和 HTTP 模式
- **多语言支持** — 中文 / 英文 / 日文
- **跨平台** — Windows / Linux / macOS

---

## 🏗️ 项目架构

```
EZCAP_QT/
├── src/                  # 源代码
│   ├── ezCap.cpp/h       # 主窗口
│   ├── dllqhyccd.cpp/h   # QHYCCD SDK 封装
│   ├── threadProcessImage.cpp/h  # 图像处理线程
│   ├── liveCapThread.cpp/h       # 实时采集线程
│   ├── tempControl.cpp/h         # 温控模块
│   ├── darkFrameTool.cpp/h       # 暗场工具
│   ├── phdLink.cpp/h             # PHD2 联动
│   ├── gpsTool.cpp/h             # GPS 工具
│   ├── cfwControl.cpp/h          # 滤镜轮控制
│   ├── toolBurst.cpp/h           # 批量采集
│   ├── toolTrigger.cpp/h         # 触发模式
│   ├── imgAnalyze.cpp/h          # 图像分析
│   ├── mcpIpcServer.cpp/h        # MCP IPC 服务器
│   └── ...
├── include/              # 头文件
├── ui/                   # Qt UI 界面文件
├── language/             # 多语言翻译文件
├── mcp_server/           # MCP Node.js 服务器
├── doc/                  # 文档
├── CMakeLists.txt        # CMake 构建配置
├── EZCAP.pro             # qmake 构建配置
└── ...
```

### 技术栈
| 组件 | 版本/说明 |
|------|----------|
| Qt | 5.12.12 / 5.13.0 (MinGW 64-bit) |
| OpenCV | 4.x |
| 构建系统 | CMake 3.16+ / qmake |
| 相机 SDK | QHYCCD SDK |
| MCP Server | Node.js 18+ |

### 多线程架构
- **ThreadProcessImage** — 图像处理线程（FPS / GPS / 直方图 / 显示 / 保存 / 暗场 / 校准）
- **LiveCapThread** — 实时采集线程
- **DownloadCapThread** — 数据采集线程
- **ThreadBurstCapture** — 批量采集线程
- **ThreadTempControl** — 温控线程
- **VideoShowThread** — 视频显示线程

---

## 📦 构建与安装

### 前置依赖
- **Qt 5.12.12+** (MinGW 64-bit 推荐)
- **OpenCV 4.x**
- **CMake 3.16+**
- **QHYCCD SDK** (从 [QHYCCD 官网](https://www.qhyccd.com/) 下载)

### CMake 构建（推荐）

```bash
# 克隆仓库
git clone https://github.com/qhyccd-qxx/EZCAP_QT.git
cd EZCAP_QT

# 创建构建目录
mkdir build && cd build

# CMake 配置（设置 Qt 路径）
cmake .. -DCMAKE_PREFIX_PATH="C:/Qt/Qt5.13.0/5.13.0/mingw73_64/lib/cmake/Qt5"

# 编译
cmake --build . --config Release
```

### qmake 构建

```bash
# 使用 qmake
qmake EZCAP.pro
mingw32-make -j4
```

### 运行

```bash
# Windows
Release/EZCAP.exe

# Linux
./EZCAP
```

---

## 🔧 MCP Server

EZCAP 内置 MCP (Model Context Protocol) 服务器，支持通过 IPC 管道或 HTTP 与外部程序通信。

### 启用方式

```bash
# 环境变量方式
set EZCAP_MCP_IPC=1
EZCAP.exe

# 命令行方式
EZCAP.exe --mcp-ipc
```

### Node.js MCP Server

```bash
cd mcp_server
npm install
npm start          # IPC 模式
npm run start:http # HTTP 模式 (默认端口 3333)
```

### 支持的命令
- `app.ping` — 心跳检测
- `app.info` — 获取应用信息
- `camera.status` — 获取相机状态
- `help.methods` — 查看可用命令

更多详情请查看 [mcp_server/README.md](mcp_server/README.md)

---

## 🌍 多语言支持

| 语言 | 文件 |
|------|------|
| 中文 (简体) | `language/lan_zh_cn.ts` |
| English | `language/lan_en_us.ts` |
| 日本語 | `language/lan_ja_jp.ts` |

---

## 📄 文档

- [用户手册 (PDF)](doc/EZCAP_QT%20User%20Manual.pdf)
- [用户手册 (Word)](doc/EZCAP_QT%20User%20Manual.docx)

---

## 📂 目录说明

| 目录 | 说明 |
|------|------|
| `src/` | 源代码 |
| `include/` | 头文件 |
| `ui/` | Qt Designer UI 文件 |
| `language/` | 多语言翻译文件 |
| `mcp_server/` | MCP Node.js 服务器 |
| `doc/` | 文档 |
| `qss/` | Qt 样式表 |
| `script/` | 脚本文件 |
| `image/` | 资源图片 |
| `winlib/` | Windows 依赖库 |
| `Depend/` | 第三方依赖（构建时生成） |
| `build/` | 构建输出（已加入 .gitignore） |

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

1. Fork 本仓库
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 提交 Pull Request

---

## 📜 许可证

本项目采用 [MIT](LICENSE) 许可证。

---

## 🔗 相关链接

- [QHYCCD 官网](https://www.qhyccd.com/)
- [QHYCCD SDK 文档](https://www.qhyccd.com/)
- [PHD2 导星软件](https://www.phd2-guiding.org/)
- [OpenCV](https://opencv.org/)
- [Qt](https://www.qt.io/)

---

**Made with ❤️ for Astronomy**

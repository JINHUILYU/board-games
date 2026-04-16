# board-games

一个基于 C++17 的命令行棋类游戏平台，当前已实现五子棋（人人对战 / 人机对战 / 历史回放），并预留了中国象棋（固定 10x9 棋盘）模块入口。

## 环境要求

- CMake >= 3.16
- 支持 C++17 的编译器（如 AppleClang / Clang / GCC）

## 构建与运行

在仓库根目录执行：

```bash
cmake -S . -B build
cmake --build build
./build/chess
```

## 当前功能

1. 五子棋人人对战
2. 五子棋人机对战（AI 难度 1-3）
3. 五子棋历史对局回放
4. 五子棋棋盘大小可配置（10-19）
5. 中国象棋菜单入口（固定 10x9，当前为 CLI 占位提示）

## 历史记录

- 对局历史默认保存到：`output/history`
- 文件名格式：`gomoku_YYYYMMDD_HHMMSS.json`
- 记录字段包含：`mode`、`board_size`、`difficulty`、`result`、`moves`

## 项目结构

```text
.
├── include/
│   ├── ai/                 # AI 接口与实现声明
│   ├── core/               # 核心抽象（状态、玩家、回合引擎、类型）
│   ├── games/              # 各棋种状态接口
│   ├── storage/            # 历史存储接口
│   └── ui/                 # 菜单与控制台玩家接口
├── src/
│   ├── ai/
│   ├── core/
│   ├── games/
│   ├── storage/
│   └── ui/
├── main.cpp                # 程序入口
└── CMakeLists.txt
```

## 说明

- 当前仓库没有配置自动化测试目标（CTest）。
- `build/` 与 `output/` 为运行产物目录，不建议纳入版本管理。
- 当前交互是命令行界面（CLI），图形界面不在当前版本范围内。

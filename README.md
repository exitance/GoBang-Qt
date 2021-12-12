# 程序设计与计算导论大作业-五子棋游戏（ Qt + C++ 实现）

SDU 2021 程序设计与计算导论大作业

## 功能

- PVE, EVE, PVP 模式
- AI 使用带 Alpha-Beta 剪枝的 MIN-MAX 算法
- 正常中止的棋局历史回放

## 环境

- Qt 版本：6.2.2
- 编译工具：CMake + mingw-w64 v9.0.0

## Windows 下打包发布

Qt Creator 中使用 Release 模式编译项目 （注意自定义路径），将生成的可执行文件 GoBang.exe 复制到发布文件夹中

打开 powershell，在发布文件夹中使用 `windeployqt` 命令

```powershell
windeployqt GoBang.exe
```

得到自动复制的插件文件夹和 dll文件、qm文件：完整的 exe 程序发布集合，依赖关系都解决好了。

## 版本

- 2021-12-12: v.1.0.0
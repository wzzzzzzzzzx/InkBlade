# 墨刃 MVP

《墨刃》是一个 2D 水墨武侠博弈格斗游戏原型，使用 C++17 和 CMake 构建。

当前默认版本使用无外部依赖的 Win32/GDI 渲染器，方便在没有网络的 Windows 环境中直接构建和打包。SFML 版本源码保留在 `src/main.cpp`，后续网络可用或本地提供 SFML 后，可通过 `-DINKBLADE_USE_SFML=ON` 启用。

## 操作说明

- `WASD`：移动
- `Space`：跳跃
- `鼠标左键`：点击为平A，长按后松开为蓄力攻击
- `G`：振刀
- `Left Shift`：闪避
- `F`：角色小技能
- `V`：能量满时释放大招
- `方向键` / `WASD`：菜单选择
- `鼠标点击` / `Enter`：确认菜单按钮
- `Esc`：返回菜单

## 构建

使用 Visual Studio 自带的 CMake 和 Ninja：

```powershell
.\scripts\build.ps1
```

默认构建不下载依赖。若后续要尝试 SFML 版本，可在 CMake 配置中添加 `-DINKBLADE_USE_SFML=ON`；该路径会通过 CMake FetchContent 下载 SFML 2.6.2。

## 打包部署

```powershell
.\scripts\package.ps1
```

可运行版本会生成在：

```text
dist\InkBlade\InkBlade.exe
```

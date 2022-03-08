# Clicorn

Alicorn PE 后端程序。

## 构建

### 准备工作

若要构建 Clicorn，除了参考 [AlicornPE](https://github.com/Andy-K-Sparklight/AlicornPE) 中的内容以外，这里有一些值得注意的细节。我们默认读者已经掌握基本的 C 程序编译方法，所以这里只做简要说明：

- 所有操作都将在一台 64 位 GNU/Linux 操作系统上完成，尽管（如果由于幸运或努力的因素）使用 Windows 进行编译是有可能的，但我们的构建工具并未进行兼容测试，因此很可能出现问题。你需要自己解决它们，通常这意味着*设法*正确配置 MinGW。

- 安装 Autotools、GNU GCC 编译器套件、GNU Make、MinGW GCC 交叉编译器。当然，你也需要 AlicornPE 中提及的一些构建工具。你还需要 Strip（在 BinUtils 中）、UPX 和 [Bin2Header](https://github.com/AntumDeluge/bin2header/releases/latest)。请将这些工具都设置好并包括到环境变量中。

- 安装库（其中一些可能已经存在于你的操作系统上）：
  
  - GTK 3.0（仅 GNU/Linux）
  
  - Webkit2GTK 4.0（仅 GNU/Linux）
  
  - LibCurl（Windows 版 DLL 已经

### 构建 Alicorn PE

首先请根据 [AlicornPE](https://github.com/Andy-K-Sparklight/AlicornPE) 仓库中的内容，完成 Alicorn PE 的构建，完成之后（已经复制好了 `pack.zip`）再回到本仓库。现在你应该位于 `AlicornPE` 的上一级目录，`Clicorn` 和 `AlicornPE` 都在此目录中。

### 编译 Clicorn

1. 进入本仓库，运行生成文件：
   
   ```shell
   cd Clicorn
   ./autogen.sh
   ```

2. 运行配置设置：
   
   ```shell
   ./configure # 如果正为 GNU/Linux 编译
   ./configure --host=x86_64-w64-mingw32 # 如果正为 Windows 交叉编译
   ```

3. 编译：
   
   ```shell
   make CFLAGS=-O2 CXXFLAGS=-O2
   ```

现在你可以在根目录下找到 `Clicorn`（ELF）或`Clicorn.exe`（MS/DOS）。

如果你只为 GNU/Linux 编译，那么，构建过程到此结束。你可以将 `Clicorn` 重命名为你喜欢的名字，然后使用它。

如果你正在为 Windows 进行编译，那么，**还没有结束**，`Clicorn.exe` 的运行需要两个动态库，而我们尚未打包它们。

### 打包 Alicorn PE

现在，运行打包脚本，以生成一个用于检查 DLL 和启动 Clicorn 的最终程序：

```shell
./winpack.sh
```

`AlicornPE.exe` 将位于根目录下，现在就可以使用它了！

# 

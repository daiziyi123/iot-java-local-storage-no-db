[![CMock 单元测试](https://github.com/FreeRTOS/FreeRTOS-Kernel/actions/workflows/unit-tests.yml/badge.svg?branch=main&event=push)](https://github.com/FreeRTOS/FreeRTOS-Kernel/actions/workflows/unit-tests.yml?query=branch%3Amain+event%3Apush+workflow%3A%22CMock+Unit+Tests%22++)
[![codecov](https://app.codecov.io/gh/FreeRTOS/FreeRTOS-Kernel/badge.svg?branch=main)](https://codecov.io/gh/FreeRTOS/FreeRTOS-Kernel)

## 快速开始

本仓库仅包含 FreeRTOS 内核的源文件/头文件以及内核移植代码。本仓库作为子模块被引用在
[FreeRTOS/FreeRTOS](https://github.com/FreeRTOS/FreeRTOS)
仓库中，该仓库在 `FreeRTOS/Demo` 目录下包含了预配置好的示例应用工程。

使用 FreeRTOS 最简单的方式是从一个预配置好的示例应用工程开始。
这样你就能确保已包含正确的 FreeRTOS 源文件，并配置好了正确的头文件包含路径。
当示例应用能够编译并运行后，你可以删除示例应用文件，然后开始添加自己的应用源文件。
详细说明和其他有用链接请参阅
[FreeRTOS 内核快速入门指南](https://www.freertos.org/Documentation/01-FreeRTOS-quick-start/01-Beginners-guide/02-Quick-start-guide)。

此外，关于 FreeRTOS 内核功能的信息，请参考
[开发者文档](https://www.freertos.org/Documentation/02-Kernel/02-Kernel-features/00-Developer-docs)
和 [API 参考](https://www.freertos.org/Documentation/02-Kernel/04-API-references/01-Task-creation/00-TaskHandle)。

关于贡献代码和创建 Pull Request，请参考
[此处的说明](.github/CONTRIBUTING.md#contributing-via-pull-request)。

**FreeRTOS-Kernel V11.1.0
[源代码](https://github.com/FreeRTOS/FreeRTOS-Kernel/tree/V11.1.0) 是
[FreeRTOS 202406.00 LTS](https://github.com/FreeRTOS/FreeRTOS-LTS/tree/202406-LTS)
版本的一部分。**

### 获取帮助

如果你有任何问题或需要帮助排查 FreeRTOS 项目，我们有一个活跃的社区可以在
[FreeRTOS 社区支持论坛](https://forums.freertos.org) 上为你提供帮助。

## 如何使用 FreeRTOS-Kernel

### 通过 CMake 使用

如果使用 CMake，建议通过 FetchContent 来使用本仓库。
将以下内容添加到你工程的主 CMakeLists.txt 或子目录的 CMakeLists.txt 中：

- 定义你要使用的源和版本/标签：

```cmake
FetchContent_Declare( freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG        main #注意：最佳实践是使用具体的 git 哈希值或带标签的版本
)
```

如果你更倾向于将其作为 git 子模块添加，请执行：

```bash
git submodule add https://github.com/FreeRTOS/FreeRTOS-Kernel.git <子模块路径>
git submodule update --init
```

- 添加一个 freertos_config 库（通常是 INTERFACE 库），以下假设目录结构为：
  - `include/FreeRTOSConfig.h`

```cmake
add_library(freertos_config INTERFACE)

target_include_directories(freertos_config SYSTEM
INTERFACE
    include
)

target_compile_definitions(freertos_config
  INTERFACE
    projCOVERAGE_TEST=0
)
```

如果你将 FreeRTOS-Kernel 作为子模块安装，则需要将其作为子目录添加：

```cmake
add_subdirectory(${FREERTOS_PATH})
```

- 配置 FreeRTOS-Kernel 并使其可用
  - 此示例同时支持本地编译和交叉编译选项。

```cmake
set( FREERTOS_HEAP "4" CACHE STRING "" FORCE)
# 选择本地编译的移植层
set( FREERTOS_PORT "GCC_POSIX" CACHE STRING "" FORCE)
# 选择交叉编译的移植层
if (CMAKE_CROSSCOMPILING)
  set(FREERTOS_PORT "GCC_ARM_CA9" CACHE STRING "" FORCE)
endif()

FetchContent_MakeAvailable(freertos_kernel)
```

- 如果是交叉编译，你还需要在 `freertos_config` 中添加以下内容：

```cmake
target_compile_definitions(freertos_config INTERFACE ${definitions})
target_compile_options(freertos_config INTERFACE ${options})
```

### 独立使用 — 克隆本仓库

使用 HTTPS 克隆：

```
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git
```

使用 SSH：

```
git clone git@github.com:FreeRTOS/FreeRTOS-Kernel.git
```

## 仓库结构

- 仓库根目录包含三个所有移植层通用的文件——list.c、queue.c 和 tasks.c。
  内核就包含在这三个文件中。croutine.c 实现了可选的协程功能——
  通常只在内存非常有限的系统上使用。

- `./portable` 目录包含针对特定微控制器和/或编译器的文件。
  更多信息请参阅 `./portable` 目录下的 readme 文件。

- `./include` 目录包含实时内核的头文件。

- `./template_configuration` 目录包含一个示例 `FreeRTOSConfig.h`，用于帮助快速启动新项目。
  说明请参阅 [FreeRTOSConfig.h](examples/template_configuration/FreeRTOSConfig.h) 文件。

### 代码格式化

FreeRTOS 文件使用
"[uncrustify](https://github.com/uncrustify/uncrustify)" 工具进行格式化。
uncrustify 使用的配置文件可在
[FreeRTOS/CI-CD-GitHub-Actions](https://github.com/FreeRTOS/CI-CD-Github-Actions)
仓库的
[uncrustify.cfg](https://github.com/FreeRTOS/CI-CD-Github-Actions/tree/main/formatting)
文件中找到。

### 行尾符

为了与 git 获得最佳兼容性，提交到 FreeRTOS-Kernel 仓库的文件使用 Unix 风格的 LF 行尾符。

为了与 Microsoft Windows 工具获得最佳兼容性，最好启用 git 的 autocrlf 功能。
你可以使用以下命令为当前仓库启用此设置：

```
git config core.autocrlf true
```

### Git 历史优化

本仓库中的某些提交进行了大规模重构，涉及大量行的修改，
在使用 `git blame` 命令时会导致不必要的行为。
你可以使用以下命令配置 git 来忽略本仓库中的大规模重构提交列表：

```
git config blame.ignoreRevsFile .git-blame-ignore-revs
```

### 拼写和格式化

我们建议使用 [Visual Studio Code](https://code.visualstudio.com)
（通常称为 VSCode）来处理 FreeRTOS-Kernel。
FreeRTOS-Kernel 还使用 [cSpell](https://cspell.org/) 作为拼写检查的一部分。
其配置文件可在 [cspell.config.yaml](cspell.config.yaml) 找到。
此外还有一个
[VSCode 的 cSpell 插件](https://marketplace.visualstudio.com/items?itemName=streetsidesoftware.code-spell-checker)
也可以使用。
*[.cSpellWords.txt](.github/.cSpellWords.txt)* 包含了传统英语词典中找不到的单词。
拼写检查器使用它来验证 FreeRTOS 代码库中使用的各种术语、变量名和其他特殊单词是否正确。
如果你的 Pull Request 未能通过拼写检查且你认为这是错误的，
请将该单词添加到 *[.cSpellWords.txt](.github/.cSpellWords.txt)* 中。
添加单词后请对列表排序，可以通过运行 bash 命令完成：
`sort -u .cSpellWords.txt -o .cSpellWords.txt`
注意，目前仅对 FreeRTOS-Kernel 源文件、[include](include)、
[portable/MemMang](portable/MemMang) 和 [portable/Common](portable/Common)
文件进行拼写和格式化检查。

## 第三方工具

有关支持 FreeRTOS 的第三方工具的详细信息，请访问
[此链接](.github/third_party_tools.md)。

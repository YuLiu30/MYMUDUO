## 项目描述

本项目模拟了 Muduo 网络库，使用 C++11 的新特性实现了一个基于非阻塞 IO 复用的高并发 TCP 服务器模型核心 `TcpServer`。主要模块包括：

- `Acceptor`
- `EventThreadPool`
- `Channel`
- `Poller`
- `EventLoop`
- `Buffer`
- `TcpConnection`
- `Logger`

主要特性：

- 整体采用 `non-blocking + IO multiplexing + loop 线程` 的设计框架。
- 线程模型使用 **one loop per thread** 的多线程服务端网络编程模型，结合 **Reactor 模型** 实现。
- 去除了 Muduo 库中的 Boost 依赖，完全使用 C++ 标准库。
- 使用 `unique_ptr`、`shared_ptr`、`weak_ptr` 管理资源，例如 `Poller`、`Channel` 等模块的内存释放，以及 `TcpConnection` 与 `Channel` 的绑定。
- 利用 `std::atomic` 保护状态变量，使用 `unique_lock` 替代 `lock_guard` 实现更灵活的同步。
- 缓冲区设计借鉴 Netty，使用 `prepend`、`read`、`write` 三个标志划分数据区域。

## 项目收获

- 通过深入阅读 Muduo 核心模块 `TcpServer` 的源码，对经典的五种 IO 模型和 Reactor 模型有了更深理解。
- 掌握了基于事件驱动和事件回调的 `epoll + 线程池` 面向对象编程。
- 熟悉了网络编程中的线程池、缓冲区管理，进一步提升了多线程编程能力。

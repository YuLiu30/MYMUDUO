# ReactorNet - C++11实现的高性能网络库

[![C++11](https://img.shields.io/badge/C++-11-blue.svg)](https://en.cppreference.com/w/cpp/11)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## 📌 项目概述
模拟Muduo网络库核心架构，使用现代C++11特性实现的Reactor模式网络库，具备以下特点：
- 完全去Boost依赖，纯标准库实现
- 非阻塞IO+多路复用+线程池的高并发模型
- 精细化的模块化设计

## 🏗️ 核心模块
| 模块               | 功能描述                                                                 |
|--------------------|--------------------------------------------------------------------------|
| `EventLoop`        | 事件循环核心，采用one loop per thread模型                                |
| `Poller`           | 基于epoll的IO多路复用实现(ET模式)                                       |
| `Channel`          | 文件描述符事件分发器，内置非阻塞IO回调                                  |
| `Buffer`           | Netty式动态缓冲区设计，支持prepend/read/write三区划分                   |
| `TcpConnection`    | 连接生命周期管理，内置智能指针自动回收                                   |

## ⚙️ 技术实现
### 现代C++特性应用
```cpp
// 智能指针管理
std::unique_ptr<Poller> poller_;  // 独占资源
std::shared_ptr<Channel> channel_; // 共享资源
std::weak_ptr<TcpConnection> conn_; // 观察者模式

// 线程安全控制
std::atomic<bool> running_{false};
std::mutex mutex_;
std::unique_lock<std::mutex> lock(mutex_);

高性能设计
plaintext
  +---------------------+
  |    Thread Pool      |
  +----------+----------+
             |
  +----------v----------+
  |   Main Reactor      |
  | (Acceptor Thread)   |
  +----------+----------+
             |
  +----------v----------+
  |   Sub Reactors      |
  | (IO Threads)        |
  +---------------------+
🎯 项目收获
深入理解：

Reactor模型与五种IO模型对比

事件驱动架构的本质实现

线程安全与锁优化实践

关键技术掌握：

epoll ET模式编程要点

环形缓冲区设计技巧

多线程任务调度策略

工程能力提升：

现代C++工程实践能力

高性能网络编程能力

复杂系统调试技巧

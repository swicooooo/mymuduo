#pragma once

#include <functional>
#include <memory>

class Buffer;
class TcpConnection;
class Timestamp;

using TcpConnctionPtr = std::shared_ptr<TcpConnection>;
using ConnectionCallbck = std::function<void(const TcpConnctionPtr&)>;
using CloseCallbck = std::function<void(const TcpConnctionPtr&)>;
using WriteCompleteCallbck = std::function<void(const TcpConnctionPtr&)>;
using MessageCallback = std::function<void(const TcpConnctionPtr&, Buffer*, Timestamp)>;
using HighWaterMarkCallback = std::function<void(const TcpConnctionPtr&, std::size_t)>;
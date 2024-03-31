#pragma once

#include <functional>
#include <memory>

class Buffer;
class TcpConnction;
class Timestamp;

using TcpConnctionPtr = std::shared_ptr<TcpConnction>;
using ConnectionCallbck = std::function<void(const TcpConnctionPtr&)>;
using CloseCallbck = std::function<void(const TcpConnctionPtr&)>;
using WriteCompleteCallbck = std::function<void(const TcpConnctionPtr&)>;
using MessageCallback = std::function<void(const TcpConnctionPtr&, Buffer, Timestamp)>;
#pragma once
#include <../external/asio/asio.hpp>
#include "handler_allocator.hpp"
#include "cyclic_buffer.hpp"
#include <atomic>

class session
    : public std::enable_shared_from_this<session>
{
    using shared_type = std::shared_ptr<session>;
public:
    explicit session(asio::ip::tcp::socket socket)
        : socket_(std::move(socket))
        , strand_(socket_.get_io_context())
        , read_buffer_(1024)
        , write_buffer_(1024)
        , state_(state_none)
    {
    }
    ~session()
    {
        assert( (state_ == state_none) || (state_ == state_stopped) );
    }

    void on_connect()
    {
        state_ = state_connected;
    }
    void async_stop()
    {
        state_ = state_async_stop;
        auto self(shared_from_this());
        socket_.get_io_context().post([this, self]() 
        {
            stop();
        });
    }
    void async_start(asio::ip::tcp::resolver::results_type endpoints)
    {
        state_ = state_async_connect;
        auto self(shared_from_this());
        strand_.post( [this, self, endpoints]()
        {
            asio::async_connect(socket_, endpoints, [this](std::error_code ec, auto ep)
            {
                if (ec)
                {
                    return;
                }
            });
        });
    }

    void send(char* data, std::size_t length)
    {
        cyclic_buffer::mutable_buffers_type send_buffer = write_buffer_.prepared();
        size_t buf_size = asio::buffer_size(send_buffer);

        if (buf_size < length)
        {
            assert(0);//关闭连接
            async_stop();
            return;
        }
        if (state_connected == state_)
        {
            std::size_t write_size = asio::buffer_copy(send_buffer, asio::buffer(data, length));
            assert(write_size == length);
            write_buffer_.consume(length);
        }
        
        // 发送请求的投递
        start_write();
    }

    // handler
    template<typename Handler>
    void reg_handler_connect(Handler h)
    {
        handler_connect_ = h;
    }

    template<typename Handler>
    void reg_handler_stopped(Handler h)
    {
        handler_stopped_ = h;
    }

    template<typename Handler>
    void reg_handler_recv(Handler h)
    {
        handler_recv_ = h;
    }
private:
    // finally stop
    void stop()
    {
        if (state_stopped == state_) return;
        state_ = state_stopped;
        std::error_code ec;
        socket_.close(ec);
    }

    std::error_code shutdown_socket()
    {

        std::error_code ec;
        socket_.shutdown(socket_.shutdown_send, ec);
        return ec;
    }
    void start_read()
    {
        auto self(shared_from_this());
        asio::streambuf::mutable_buffers_type read_data = read_buffer_.prepare(read_buffer_.capacity() - read_buffer_.size());
        if (read_data.size() == 0) return;

        socket_.async_read_some(read_data, 
            make_custom_alloc_handler(read_allocator_,
            [this, self](std::error_code ec, std::size_t length)
        {
            if (asio::error::eof == ec)
            {
                shutdown_socket();
            }
            if (ec) 
            {
                stop();
                return;
            }
            read_buffer_.commit(length);
            if (handler_recv_)
            {
                handler_recv_(shared_from_this(), read_buffer_);
            }
            start_read();
        }));
    }
    void start_write()
    {
        if (is_writing_) return;
        is_writing_ = true;
        auto self(shared_from_this());
        cyclic_buffer::const_buffers_type write_data = write_buffer_.data();
        if (write_data.empty()) return;

        asio::async_write(socket_, write_data,
            make_custom_alloc_handler(write_allocator_,
                [this, self](std::error_code ec, std::size_t length) 
        {
            if (ec)
            {
                stop();
            }
            write_buffer_.commit(length);

            is_writing_ = false;
            start_write();
        }));
    }
    enum state
    {
        state_none,
        state_async_connect,
        state_async_accept,
        state_connected,
        state_async_stop,
        state_shutdown,
        state_stopped,
    };
    state state_ = state_none;
    std::atomic<bool> is_writing_ = false;
    handler_allocator<512> read_allocator_;
    handler_allocator<512> write_allocator_;

    asio::streambuf read_buffer_;
    cyclic_buffer write_buffer_;
    asio::ip::tcp::socket socket_;
    asio::io_service::strand strand_;

    //handler
    std::function<void (shared_type s)> handler_connect_;
    std::function<void (shared_type s)> handler_stopped_;
    std::function<void (shared_type s, asio::streambuf& buffer)> handler_recv_;
};
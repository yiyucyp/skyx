/****************************************************************************
*
*  brief
*  对asio tcp server
*  必须定义ASIO_STANDALONE宏，完全独立地BOOST了
*  data: 2017-08-22
*  author: caiyp
*****************************************************************************/

#pragma once
#include <thread>
#include <../external/asio/asio.hpp>


//template<typename session_type>
class asio_server
{
public:
    asio_server(short port, int thread_count = 1) 
        : acceptor_(io_context_
            , asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        , thread_count_(thread_count)
    {
    }

    template<typename Handler>
    void reg_handler_accept(Handler h)
    {
        handler_on_accept_ = h;
    }
    void start()
    {
        do_accept();
        run_thread();
    }
    void stop() 
    {
        io_context_.stop();
        for (std::thread& t : thread_pool_)
        {
            if(t.joinable()) t.join();
        }
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](std::error_code ec, asio::ip::tcp::socket socket)
        {
            if (!ec && handler_on_accept_)
            {
                //auto session = std::make_shared<session_type>(std::move(socket));
                //session->on_connect();
                handler_on_accept_(std::move(socket));
            }
            do_accept();
        });
    }

    void run_thread()
    {
        for (int i = 0; i < thread_count_; ++i)
        {
            thread_pool_.emplace_back([=]()
            {
                io_context_.run();
            });
        }
    }
    std::function<void(asio::ip::tcp::socket socket)> handler_on_accept_;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;
    std::vector<std::thread> thread_pool_;
    int thread_count_;
};
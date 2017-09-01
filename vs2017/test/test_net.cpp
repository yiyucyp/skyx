#include "net/asio_server.hpp"
#include "net/session.hpp"
#include "net/handler_allocator.hpp"
#include "net/cyclic_buffer.hpp"
#include <list>

struct gate_client
{
    static std::size_t read_size_;
    static std::size_t write_size_;

    void on_connect() {

    }
    void on_read() {

    }

    void on_close() {

    }
};

class gate_session_mgr 
{
    typedef session client_type;
    using shared_client = std::shared_ptr<client_type>;
    //friend class asio_server;
public:
   


    void on_connect(shared_client s)
    {
        data_.push_back(s);
    }

    void on_stopped(shared_client s)
    {
        auto it = std::find(data_.begin(), data_.end(), s);
        if (it != data_.end()) data_.erase(it);
    }

    void on_recv(shared_client s, asio::streambuf& buffer)
    {
        // 读完要commint


        buffer.consume(10);
        //asio::buffered_read_stream
    }

private:
    std::list<shared_client> data_;
};

gate_session_mgr g_session_mgr;

void server_run()
{
    {
        size_t ret = 0;
        size_t buf_size = 0;
        cyclic_buffer buf(32);
        auto mus= buf.prepared();
        buf_size = asio::buffer_size(mus);
        ret = asio::buffer_copy(mus, asio::buffer("123456"));
        buf.consume(ret);
        mus = buf.prepared();
        buf_size = asio::buffer_size(mus);
        ret = asio::buffer_copy(mus, asio::buffer("78910"));
        buf.consume(ret);
        mus = buf.prepared();   
        buf_size = asio::buffer_size(mus);
        ret = asio::buffer_copy(mus, asio::buffer("abcdefhjklmnopqrstuvwxyz"));
        buf.consume(ret);
        mus = buf.prepared();  
        buf_size = asio::buffer_size(mus);
        ret = asio::buffer_copy(mus, asio::buffer("12345678910"));
        buf.consume(ret);
        mus = buf.prepared();  
        buf_size = asio::buffer_size(mus);

        //asio::buffered_read_stream;


    }
    //asio::streambuf
    {
        asio::streambuf oo(104);
        auto  b = oo.prepare(oo.capacity());
        asio::buffer_copy(b, asio::buffer("123456"));

        oo.commit(6);
        auto c = oo.capacity();
        auto s = oo.size();
        oo.consume(4);
        c = oo.capacity();
        s = oo.size();
        //oo.read_size_helper();

    }


    asio_server server(10086, 1);
    server.reg_handler_accept([=](asio::ip::tcp::socket socket) 
    {
        auto s = std::make_shared<session>(std::move(socket));
        s->reg_handler_connect(std::bind(&gate_session_mgr::on_connect, &g_session_mgr, std::placeholders::_1));
        s->reg_handler_stopped(std::bind(&gate_session_mgr::on_stopped, &g_session_mgr, std::placeholders::_1));
        s->reg_handler_recv(std::bind(&gate_session_mgr::on_recv, &g_session_mgr, std::placeholders::_1, std::placeholders::_2));
    });

    server.start();

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
}
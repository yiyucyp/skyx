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
public:

    std::list<shared_client> data_;
};

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



    }

    handler_allocator<512> read_allocator_;

    auto hh = make_custom_alloc_handler(read_allocator_, []() {});
    asio_server<session> server(10086, 1);

}
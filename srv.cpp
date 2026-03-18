#include <iostream>
#include <set>
#include <deque>
#include <string>
#include <memory>
#include <boost/asio.hpp>

using namespace boost;
using asio::ip::tcp;


/* 

  model class Instance_Subscriber
  config class Room_Config
  handle_client class Handle_Client

*/

class Instance_Subscriber{
  public:
    virtual ~Instance_Subscriber(){}
    virtual void deliver(const std::string& msg) = 0;
};

class Room_Config{
  public:
    void join(std::shared_ptr<Instance_Subscriber> Subscriber_){
      range_all_subscriber.insert(Subscriber_);
    }

    void deliver(const std::string& msg){
      for(auto range_subscriber : range_all_subscriber){
        range_subscriber->deliver(msg);
      }
    }

  private:
    std::set<std::shared_ptr<Instance_Subscriber>> range_all_subscriber;
    std::deque<std::string> str_msg;
};

class Handle_Client : public Instance_Subscriber, 
public std::enable_shared_from_this<Handle_Client> {
  public:
    Handle_Client(tcp::socket socket, Room_Config& room) : socket_(std::move(socket)), 
      room_(room), timer_(socket.get_executor()){
    }

    void start(){
      room_.join(shared_from_this());

      asio::co_spawn(socket_.get_executor(), 
        [self = shared_from_this()]{return self->reader();}, asio::detached);

      asio::co_spawn(socket_.get_executor(), 
        [self = shared_from_this()]{return self->writer();}, asio::detached);
    }

    void deliver(const std::string& msg){
      str_msg_handle.push_back(msg);

      /*
      --------------------------------------------
        server log:
      */
      std::string server_msg;
      std::cout <<" log: " << msg;
    }
    
  private:
    /*
      (awaitable<void> writer and awaitable<void> reader)
      -> private class
    */
    asio::awaitable<void> reader(){
      for(std::string buf;;){
        size_t n = co_await asio::async_read_until(socket_, asio::dynamic_buffer(buf, 1024), 
          '\n', asio::use_awaitable);

          room_.deliver(buf.substr(0, n));
          buf.erase(0, n);
      }
    }
    asio::awaitable<void> writer(){
      while(socket_.is_open()){
        if(str_msg_handle.empty()){
          /*
            this solution: 
              timer.async_wait(...)
          */
          system::error_code ec;
          co_await timer_.async_wait(asio::redirect_error(asio::use_awaitable, ec)); 
        }else{
          co_await asio::async_write(socket_, 
            asio::buffer(str_msg_handle.front()), 
            asio::use_awaitable);
          str_msg_handle.pop_front();
          }
        }
    }
    Room_Config& room_;
    tcp::socket socket_;
    asio::steady_timer timer_; /*solution: for str_msg_handle alloc error*/
    std::deque<std::string> str_msg_handle;
};

namespace this_coro =  boost::asio::this_coro;
asio::awaitable<void> listen(){
  auto executor = co_await this_coro::executor;
  tcp::acceptor acceptor(executor, tcp::endpoint(tcp::v4(), 12345)); /* localhost or ip, port*/
  Room_Config room;

  for(;;){
    std::make_shared<Handle_Client>(
      co_await acceptor.async_accept(asio::use_awaitable), 
      room)->start();
  }
}

int main(){
  asio::io_context io_context;
    asio::co_spawn(io_context, listen(), asio::detached);
  io_context.run();
}
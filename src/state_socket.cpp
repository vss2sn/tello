#include "state_socket.hpp"
#include "utils.hpp"

StateSocket::StateSocket(
#ifdef USE_BOOST
  boost::asio::io_service& io_service,
#else
  asio::io_service& io_service,
#endif
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port
):
  BaseSocket(io_service, drone_ip, drone_port, local_port)
{
  #ifdef USE_BOOST
    boost::asio::ip::udp::resolver resolver(io_service_);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), drone_ip_, drone_port_);
    boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;

    socket_.async_receive_from(
      boost::asio::buffer(data_, max_length_),
      endpoint_,
      boost::bind(&StateSocket::handleResponseFromDrone,
                  this,
                  boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));

    io_thread = boost::thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));
  #else
    asio::ip::udp::resolver resolver(io_service_);
    asio::ip::udp::resolver::query query(asio::ip::udp::v4(), drone_ip_, drone_port_);
    asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
    endpoint_ = *iter;

    socket_.async_receive_from(
      asio::buffer(data_, max_length_),
      endpoint_,
      [&](const std::error_code& error, size_t bytes_recvd)
      {return handleResponseFromDrone(error, bytes_recvd);});
      // [&](auto... args){return handleResponseFromDrone(args...);});

      io_thread = std::thread([&]{io_service_.run();});
      io_thread.detach();

  #endif
}

#ifdef USE_BOOST
void StateSocket::handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd)
#else
void StateSocket::handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd)
#endif
{
  if(!error && bytes_recvd>0){
    response_ = std::string(data_);
    std::replace(response_.begin(), response_.end(), ';', '\n');
    // std::cout << "Status: \n" << response_ << std::endl;
  }
  else{
    // utils_log::LogDebug() << "Error/Nothing received" ;
  }

#if USE_BOOST
 socket_.async_receive_from(
   boost::asio::buffer(data_, max_length_),
   endpoint_,
   boost::bind(&StateSocket::handleResponseFromDrone,
     this,
     boost::asio::placeholders::error,
     boost::asio::placeholders::bytes_transferred));
#else
  socket_.async_receive_from(
    asio::buffer(data_, max_length_),
    endpoint_,
    [&](const std::error_code& error, size_t bytes_recvd)
    {return handleResponseFromDrone(error, bytes_recvd);});
    // [&](auto... args){return handleResponseFromDrone(args...);});
#endif
}

StateSocket::~StateSocket(){
  socket_.close();
}

#ifdef USE_BOOST
void StateSocket::handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, std::string cmd)
#else
void StateSocket::handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd)
#endif
{
  std::cout << "StateSocket class does not implement handleSendCommand()" << std::endl;
}

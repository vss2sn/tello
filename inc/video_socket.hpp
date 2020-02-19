#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <chrono>
#include <vector>
#include <cstring>

#include <libavutil/frame.h>
#include <opencv2/highgui.hpp>

#include "utils.hpp"
#include "h264decoder.hpp"

class VideoSocket{
public:

  VideoSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
  ~VideoSocket();

private:

  void handleResponseFromDrone(const boost::system::error_code& error, size_t r);
  void decodeFrame();

  enum{ max_length_ =  2048 };
  enum{ max_length_large_ =  65536 };
  bool received_response_ = true;

  char data_[max_length_];
  char frame_buffer_[max_length_large_];

  std::string response_;
  std::string drone_ip_, drone_port_, local_port_;
  boost::asio::io_service& io_service_;
  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::endpoint endpoint_;

  size_t first_empty_index = 0;
  int frame_buffer_n_packets_ = 0;

  H264Decoder decoder_;
  ConverterRGB24 converter_;
};

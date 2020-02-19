#ifndef VIDEOSOCKET_H
#define VIDEOSOCKET_H

#include <chrono>
#include <vector>
#include <cstring>

#include <libavutil/frame.h>
#include <opencv2/highgui.hpp>

#include "base_socket.hpp"
#include "h264decoder.hpp"

class VideoSocket  : public BaseSocket{
public:

  VideoSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port);
  ~VideoSocket();

private:

  void handleResponseFromDrone(const boost::system::error_code& error, size_t r) override;
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, const std::string& cmd) override;
  void decodeFrame();

  enum{ max_length_ =  2048 };
  enum{ max_length_large_ =  65536 };
  bool received_response_ = true;

  char data_[max_length_];
  char frame_buffer_[max_length_large_];

  size_t first_empty_index = 0;
  int frame_buffer_n_packets_ = 0;

  H264Decoder decoder_;
  ConverterRGB24 converter_;
};

#endif VIDEOSOCKET_H

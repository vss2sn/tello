#ifndef VIDEOSOCKET_HPP
#define VIDEOSOCKET_HPP

#include <chrono>
#include <vector>
#include <cstring>
#include <queue>
#include <mutex>

#include <libavutil/frame.h>
#include <opencv2/highgui.hpp>

#include "base_socket.hpp"
#include "h264decoder.hpp"
// #include "openvslam_api.hpp"

#include "pangolin_viewer/viewer.h"
#include "openvslam/system.h"
#include "openvslam/config.h"
#include "openvslam/util/stereo_rectifier.h"

#include <iostream>
#include <numeric>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <spdlog/spdlog.h>
// #include <popl.hpp>

#ifdef RUN_SLAM
#include "openvslam_api.hpp"
#endif

#ifdef RUN_SLAM
#include "openvslam_api.hpp"
#endif

class VideoSocket : public BaseSocket{
public:
#ifdef USE_BOOST
  VideoSocket(boost::asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port,  bool& run);
#else
  VideoSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, bool& run);
#endif
  ~VideoSocket();

private:

#ifdef USE_BOOST
  void handleResponseFromDrone(const boost::system::error_code& error, size_t r) override;
  void handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, std::string cmd) override;
#else
  void handleResponseFromDrone(const std::error_code& error, size_t r) override;
  void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd) override;
#endif

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
  std::unique_ptr<cv::VideoWriter> video;
#ifdef RUN_SLAM
  std::unique_ptr<OpenVSLAM_API> api_;
#endif
  bool& run_;
};

#endif VIDEOSOCKET_HPP

#ifndef VIDEOSOCKET_HPP
#define VIDEOSOCKET_HPP

#include <chrono>
#include <vector>
#include <cstring>
#include <queue>
#include <mutex>
#include <ctime>
#include <atomic>

#include <libavutil/frame.h>
#include <opencv2/highgui.hpp>

#include "base_socket.hpp"
#include "h264decoder.hpp"

#include <iostream>
#include <numeric>

#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>

#ifdef RUN_SLAM
#include "openvslam_api.hpp"
#endif // RUN_SLAM

class VideoSocket : public BaseSocket{
public:

  /**
  * @brief Constructor
  * @param [in] io_service io_service object used to handle all socket communication
  * @param [in] drone_ip ip address of drone
  * @param [in] drone_port port number on the drone
  * @param [in] local_port port on the local machine used to communicate with the drone
  * @param [in] run reference to a bool that is set to off when the Tello object destructor is called
  * @return none
  */
  VideoSocket(asio::io_service& io_service, const std::string& drone_ip, const std::string& drone_port, const std::string& local_port, bool& run, std::string camera_config_file, std::string vocabulary_file);

  /**
  * @brief Destructor
  * @return none
  */
  ~VideoSocket();

  /**
  * @brief take a snapshot of the next frame
  * @return void
  */
  void setSnapshot();

private:

  void handleResponseFromDrone(const std::error_code& error, size_t r) override;
  void handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd) override;

  void decodeFrame();
  void takeSnapshot(cv::Mat& image);

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
#endif // RUN_SLAM
  bool& run_;
  std::atomic<bool> snap_ = false;
};

#endif // VIDEOSOCKET_HPP

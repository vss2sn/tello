#ifdef RUN_SLAM

#ifndef OPENVSLAM_API_HPP
#define OPENVSLAM_API_HPP

#include <memory>
#include <string>
#include <iostream>
#include <mutex>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class OpenVSLAM_API{
public:

  /**
  * @brief Constructor
  * @param [in] run reference to a bool that is set to off when the Tello object destructor is called
  * @param [in] config_file_path path to camera configuration file
  * @param [in] vocab_file_path path to vocabulary file
  * @return none
  */
  OpenVSLAM_API(bool& run, std::string config_file_path, std::string vocab_file_path);

  /**
  * @brief Destructor
  * @return none
  */
  ~OpenVSLAM_API();

  /**
  * @brief add frame to queue for frames to be processed for SLAM
  * @param [in] new_frame input image
  * @return void
  */
  void addFrameToQueue(cv::Mat new_frame);

  /**
  * @brief start monocular SLAM processing
  * @return void
  */
  void startMonoThread();

  /**
  * @brief get mutex for displaying images to multiple opencv windows
  * @return std::mutex reference to the frame_display_sync mutex
  * @details <details>
  */
  std::mutex& getMutex();

private:

  class impl;
  std::unique_ptr<impl> openvslam_impl;

};

#endif

#endif

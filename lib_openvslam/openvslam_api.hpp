#include <memory>
#include <string>
#include <iostream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class OpenVSLAM_API{
public:
  OpenVSLAM_API(bool& run, std::string config_file_path, std::string vocab_file_path);
  ~OpenVSLAM_API();
  void addFrameToQueue(cv::Mat new_frame);
  void startMonoThread();
private:
  class impl;
  std::unique_ptr<impl> openvslam_impl;
};

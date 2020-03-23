#include <queue>

#include "openvslam_api.hpp"

#include "pangolin_viewer/viewer.h"
#include "openvslam/system.h"
#include "openvslam/config.h"
#include "openvslam/util/stereo_rectifier.h"

#include <spdlog/spdlog.h>


class OpenVSLAM_API::impl{
public:
  impl(bool& run, std::string config_file_path, std::string vocab_file_path);
  void addFrameToQueue(cv::Mat new_frame);
  void startMonoThread();
  void mono_tracking(
    const std::shared_ptr<openvslam::config>& cfg,
    const std::string& mask_img_path,
    const float scale,
    const std::string& map_db_path);
private:
  std::queue<cv::Mat> frame_queue;
  std::mutex frame_m;
  std::thread mono_thread;
  bool& run_;
  std::string config_file_path_;
  std::string vocab_file_path_;
};

OpenVSLAM_API::impl::impl(bool& run, std::string config_file_path, std::string vocab_file_path)
:
run_(run),
config_file_path_(config_file_path),
vocab_file_path_(vocab_file_path)
{}

OpenVSLAM_API::OpenVSLAM_API(bool& run, std::string config_file_path, std::string vocab_file_path){
  openvslam_impl = std::make_unique<impl>(run, config_file_path, vocab_file_path);
}

OpenVSLAM_API::~OpenVSLAM_API(){};

void OpenVSLAM_API::impl::addFrameToQueue(cv::Mat new_frame){
  if(frame_queue.size() < 5){
    std::unique_lock<std::mutex> lk(frame_m);
    frame_queue.push(new_frame);
  }
}

void OpenVSLAM_API::addFrameToQueue(cv::Mat new_frame){
  openvslam_impl->addFrameToQueue(new_frame);
}

void OpenVSLAM_API::impl::mono_tracking(
  const std::shared_ptr<openvslam::config>& cfg,
  const std::string& mask_img_path,
  const float scale,
  const std::string& map_db_path){
    // load the mask image
    const cv::Mat mask = mask_img_path.empty() ? cv::Mat{} : cv::imread(mask_img_path, cv::IMREAD_GRAYSCALE);

    // build a SLAM system
    openvslam::system SLAM(cfg, vocab_file_path_);

    // startup the SLAM process
    SLAM.startup();

    pangolin_viewer::viewer viewer(cfg, &SLAM, SLAM.get_frame_publisher(), SLAM.get_map_publisher());

    cv::Mat frame;
    double timestamp = 0.0;
    std::vector<double> track_times;
    unsigned int num_frame = 0;
    // run the SLAM in another thread

    std::thread slam_thread([&]() {
        while(run_){
          if (SLAM.terminate_is_requested()) {
              break;
          }
          if(frame_queue.empty()){
            usleep(1000000.0 / cfg->camera_->fps_);
            continue;
          }

          {
            std::unique_lock<std::mutex> lk(frame_m);
            frame = frame_queue.front();
            frame_queue.pop();
          }

          if (frame.empty()) {
              continue;
          }
          const auto tp_1 = std::chrono::steady_clock::now();
          SLAM.feed_monocular_frame(frame, timestamp, mask);
          const auto tp_2 = std::chrono::steady_clock::now();
          const auto track_time = std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count();

          track_times.push_back(track_time);

          timestamp += 1.0 / cfg->camera_->fps_;
          ++num_frame;
        }
        std::cout  << "----------- Exiting the mono thread -----------"<< std::endl;
    });

    std::unique_ptr<std::thread> view = std::make_unique<std::thread>([&]{
      try{
        viewer.run();
      }
      catch(...){
        std::cout << "Viewer crashed." << std::endl;
      }
      });

    slam_thread.join();
    viewer.request_terminate();
    view->join();
    std::cout  << "----------- Viewer shutdown -----------" << std::endl;

    // shutdown the SLAM process
    SLAM.shutdown();

    if (!map_db_path.empty()) {
        // output the map database
        SLAM.save_map_database(map_db_path);
    }
    std::cout  << "----------- SLAM shutdown -----------" << std::endl;
}

void OpenVSLAM_API::impl::startMonoThread(){
  mono_thread = std::thread([&]{
    try{
      std::shared_ptr<openvslam::config> cfg = std::make_shared<openvslam::config>(config_file_path_);
      mono_tracking(cfg, "", 1, "");
    }
    catch(...){
      std::cout << "Mono thread crashed" << std::endl;
    }
  });
  mono_thread.detach();
}

void OpenVSLAM_API::startMonoThread(){
  openvslam_impl->startMonoThread();
}

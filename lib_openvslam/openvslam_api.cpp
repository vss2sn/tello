// To prevent linker language error
#include <queue>
#include <iostream>

#ifdef RUN_SLAM
#include "openvslam_api.hpp"

#include "pangolin_viewer/viewer.h"
#include "openvslam/system.h"
#include "openvslam/config.h"
#include "openvslam/util/stereo_rectifier.h"

#include <spdlog/spdlog.h>


class OpenVSLAM_API::impl{
public:
  impl(bool& run,
    const std::string config_file_path,
    const std::string vocab_file_path,
    const std::string load_map_db_path,
    const std::string save_map_db_path,
    const std::string mask_img_path,
    bool load_map,
    bool continue_mapping,
    float scale
  );
  void addFrameToQueue(cv::Mat new_frame);
  void startMonoThread();
  void mono_tracking(const std::shared_ptr<openvslam::config>& cfg);
private:
  std::queue<cv::Mat> frame_queue;
  std::mutex frame_m;
  std::thread mono_thread;
  bool& run_;
  const std::string config_file_path_;
  const std::string vocab_file_path_;
  const std::string load_map_db_path_;
  const std::string save_map_db_path_;
  const std::string mask_img_path_;
  bool load_map_ = false, continue_mapping_ = true;
  float scale_ = 1;
};


OpenVSLAM_API::impl::impl(
  bool& run,
  const std::string config_file_path,
  const std::string vocab_file_path,
  const std::string load_map_db_path,
  const std::string save_map_db_path,
  const std::string mask_img_path,
  bool load_map,
  bool continue_mapping,
  float scale
)
:
run_(run),
config_file_path_(config_file_path),
vocab_file_path_(vocab_file_path),
load_map_db_path_(load_map_db_path),
save_map_db_path_(save_map_db_path),
mask_img_path_(mask_img_path),
load_map_(load_map),
continue_mapping_(continue_mapping),
scale_(scale)
{
  if(load_map_ && !load_map_db_path.empty()){
    bool exists = false;
    if (FILE *file = fopen(load_map_db_path.c_str(), "r")) {
      fclose(file);
      exists = true;
    }
    if(!exists){
      load_map_ = false;
      std::cout << "Map file provided does not exist" << std::endl;
    }
  }
}

OpenVSLAM_API::OpenVSLAM_API(bool& run,
  const std::string config_file_path,
  const std::string vocab_file_path,
  const std::string load_map_db_path,
  const std::string save_map_db_path,
  const std::string mask_img_path,
  bool load_map,
  bool continue_mapping,
  float scale){

  openvslam_impl = std::make_unique<impl>(run, config_file_path, vocab_file_path,
    load_map_db_path, save_map_db_path, mask_img_path, load_map, continue_mapping,
    scale);
}

OpenVSLAM_API::~OpenVSLAM_API(){};

std::mutex& OpenVSLAM_API::getMutex(){
  return pangolin_viewer::frame_display_sync;
}
void OpenVSLAM_API::impl::addFrameToQueue(cv::Mat new_frame){
  if(frame_queue.size() < 3){
    std::unique_lock<std::mutex> lk(frame_m);
    frame_queue.push(new_frame.clone());
  }
}

void OpenVSLAM_API::addFrameToQueue(cv::Mat new_frame){
  openvslam_impl->addFrameToQueue(new_frame);
}

void OpenVSLAM_API::impl::mono_tracking(
  const std::shared_ptr<openvslam::config>& cfg
){
    // load the mask image
    const cv::Mat mask = mask_img_path_.empty() ? cv::Mat{} : cv::imread(mask_img_path_, cv::IMREAD_GRAYSCALE);

    // build a SLAM system
    openvslam::system SLAM(cfg, vocab_file_path_);

    if(load_map_){
      SLAM.load_map_database(load_map_db_path_);
      // startup the SLAM process (it does not need initialization of a map)
      SLAM.startup(false);
      // select to activate the mapping module or not
      if (continue_mapping_) {
        SLAM.enable_mapping_module();
      }
      else {
        SLAM.disable_mapping_module();
      }
    }
    else{
      // startup the SLAM process
      SLAM.startup();
    }

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
          auto m = SLAM.feed_monocular_frame(frame, timestamp, mask);
          
          const auto tp_2 = std::chrono::steady_clock::now();
          const auto track_time = std::chrono::duration_cast<std::chrono::duration<double>>(tp_2 - tp_1).count();

          track_times.push_back(track_time);

          timestamp += 1.0 / cfg->camera_->fps_;
          ++num_frame;
        }
         spdlog::info( "----------- Exiting the mono thread -----------");
    });

    std::unique_ptr<std::thread> view = std::make_unique<std::thread>([&]{
      try{
        spdlog::info( "----------- Viewer thread started -----------" );
        viewer.run();
      }
      catch(...){
         spdlog::info( "----------- Viewer thread crashed -----------" );
      }
      });

    slam_thread.join();
    viewer.request_terminate();
    view->join();
     spdlog::info( "----------- Viewer thread joined -----------" );

    // shutdown the SLAM process
    SLAM.shutdown();

    if (!save_map_db_path_.empty()) {
        // output the map database
        SLAM.save_map_database(save_map_db_path_);
    }
     // spdlog::info( "SLAM shutdown" );
}

void OpenVSLAM_API::impl::startMonoThread(){
  mono_thread = std::thread([&]{
    try{
      spdlog::info( "----------- Mono thread started -----------" );
      std::shared_ptr<openvslam::config> cfg = std::make_shared<openvslam::config>(config_file_path_);
      mono_tracking(cfg);
    }
    catch(...){
       spdlog::info( "----------- Mono thread crashed -----------" );
       spdlog::info( "Please check whether the config.yaml file is in the build directory" );
       spdlog::info( "Please check whether the ORB vocabulary is in the build directory" );
    }
  });
  mono_thread.detach();
}

void OpenVSLAM_API::startMonoThread(){
  openvslam_impl->startMonoThread();
}

#endif

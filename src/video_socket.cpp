#include "video_socket.hpp"
#include "utils.hpp"

VideoSocket::VideoSocket(
#ifdef USE_BOOST
  boost::asio::io_service& io_service,
#else
  asio::io_service& io_service,
#endif
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port,
  bool& run
):
  BaseSocket(io_service, drone_ip, drone_port, local_port),
  run_(run)
{
#ifdef USE_BOOST
  boost::asio::ip::udp::resolver resolver(io_service_);
  boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(), drone_ip_, drone_port_);
  boost::asio::ip::udp::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;

  socket_.async_receive_from(
    boost::asio::buffer(data_, max_length_),
    endpoint_,
    boost::bind(&VideoSocket::handleResponseFromDrone,
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

    io_thread = std::thread([&]{io_service_.run();
      LogDebug() << "----------- Video socket io_service thread exits -----------";
    });
    io_thread.detach();

    mono_thread = std::thread([&]{
      try{
        std::shared_ptr<openvslam::config> cfg = std::make_shared<openvslam::config>("./config.yaml");
        mono_tracking(cfg, "./orb_vocab.dbow2", 0, "", 1, "");
      }
      catch(...){
        LogDebug() << "Mono thread crashed";
      }
    });
    mono_thread.detach();
#endif

#ifdef RECORD
  video = std::make_unique<cv::VideoWriter>("out.mp4",cv::VideoWriter::fourcc('m','p','4','v'), 30, cv::Size(960,720));
#endif
}

#ifdef USE_BOOST
void VideoSocket::handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd)
#else
void VideoSocket::handleResponseFromDrone(const std::error_code& error, size_t bytes_recvd)
#endif
{
  if(first_empty_index == 0){
    first_empty_index = 0;
    frame_buffer_n_packets_ = 0;
  }

  if (first_empty_index + bytes_recvd >= max_length_large_) {
    LogInfo() << "Frame buffer overflow. Dropping frame";
    first_empty_index = 0;
    frame_buffer_n_packets_ = 0;
    return;
  }

  memcpy(frame_buffer_ + first_empty_index, data_, bytes_recvd );
  first_empty_index += bytes_recvd;
  frame_buffer_n_packets_++;

  if (bytes_recvd < 1460) {
    decodeFrame();
    first_empty_index = 0;
    frame_buffer_n_packets_ = 0;
  }

#if USE_BOOST
 socket_.async_receive_from(
   boost::asio::buffer(data_, max_length_),
   endpoint_,
   boost::bind(&VideoSocket::handleResponseFromDrone,
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

void VideoSocket::decodeFrame()
{
  size_t next = 0;

  try {
    while (next < first_empty_index) {
      ssize_t consumed = decoder_.parse((unsigned char*)&frame_buffer_ + next, first_empty_index - next);

      if (decoder_.is_frame_available()) {
        const AVFrame &frame = decoder_.decode_frame();
        unsigned char bgr24[converter_.predict_size(frame.width, frame.height)];
        converter_.convert(frame, bgr24);

        cv::Mat mat{frame.height, frame.width, CV_8UC3, bgr24};
#ifdef RECORD
        video->write(mat);
#endif
        {
          std::unique_lock<std::mutex> lk(m2);
          cv::Mat greyMat;
          cv::cvtColor(mat, greyMat, cv::COLOR_BGR2GRAY);
          frame_queue.push(greyMat);
          if(frame_queue.size() > 20) frame_queue.pop();
        }
        cv::imshow("frame", mat);
        cv::waitKey(1);
      }
      next += consumed;
    }
  }
  catch (...) {
    LogErr() << "Error in decoding frame";
  }
}

VideoSocket::~VideoSocket(){
#ifdef RECORD
  video->release();
#endif
  cv::destroyAllWindows();
  socket_.close();
}

#ifdef USE_BOOST
void VideoSocket::handleSendCommand(const boost::system::error_code& error, size_t bytes_sent, std::string cmd)
#else
void VideoSocket::handleSendCommand(const std::error_code& error, size_t bytes_sent, std::string cmd)
#endif
{
  LogErr() << "VideoSocket class does not implement handleSendCommand()";
}


void VideoSocket::mono_tracking(const std::shared_ptr<openvslam::config>& cfg,
                   const std::string& vocab_file_path, const unsigned int cam_num, const std::string& mask_img_path,
                   const float scale, const std::string& map_db_path) {
    // load the mask image
    const cv::Mat mask = mask_img_path.empty() ? cv::Mat{} : cv::imread(mask_img_path, cv::IMREAD_GRAYSCALE);

    // build a SLAM system
    openvslam::system SLAM(cfg, vocab_file_path);
    // startup the SLAM process
    SLAM.startup();

    pangolin_viewer::viewer viewer(cfg, &SLAM, SLAM.get_frame_publisher(), SLAM.get_map_publisher());

    cv::Mat frame;
    double timestamp = 0.0;
    std::vector<double> track_times;

    unsigned int num_frame = 0;

    // run the SLAM in another thread
    std::thread thread([&]() {
        while(run_){
          if (SLAM.terminate_is_requested()) {
              break;
          }
          if(frame_queue.empty()){
            usleep(1.0 / cfg->camera_->fps_);
            continue;
          }

          {
            std::unique_lock<std::mutex> lk(m2);
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
        // SLAM.terminate_is_requested();
        // viewer.request_terminate();
        LogDebug() << "----------- Exiting the mono thread -----------";

    });

    LogDebug() << "----------- Viewer run beginning -----------";

    std::unique_ptr<std::thread> view = std::make_unique<std::thread>([&]{
      try{
        viewer.run();
      }
      catch(...){
        LogDebug() << "Viewer crashed.";
      }
      });

    // view->detach();

    thread.join();
    LogDebug() << "----------- Viewer shutdown beginning -----------";

    viewer.request_terminate();

    // while(viewer)
    // try{
    //   LogDebug() << "Trying";
    //
    //   viewer.request_terminate();
    //   // LogDebug() << "Trying";
    //   //
    //   // // usleep(1000000);
    //   // LogDebug() << "Trying";
    //   //
    //   // viewer.~viewer();
    //   // LogDebug() << "Trying";
    //   //
    //   // view.reset();
    //   // LogDebug() << "Trying";
    //
    // }
    // catch(...){
    //   LogDebug() << "Have to find a better way.";
    // }

    view->join();
    // shutdown the SLAM process
    LogDebug() << "----------- SLAM shutdown end -----------";
    SLAM.shutdown();

    if (!map_db_path.empty()) {
        // output the map database
        SLAM.save_map_database(map_db_path);
    }

    // std::sort(track_times.begin(), track_times.end());
    // const auto total_track_time = std::accumulate(track_times.begin(), track_times.end(), 0.0);
    // std::cout << "median tracking time: " << track_times.at(track_times.size() / 2) << "[s]" << std::endl;
    // std::cout << "mean tracking time: " << total_track_time / track_times.size() << "[s]" << std::endl;
    LogDebug() << "----------- SLAM shutdown ending -----------";
}

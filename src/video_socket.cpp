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
  // cv::namedWindow("frame", CV_WINDOW_NORMAL);
  // cv::moveWindow("frame",960,0);
  // cv::resizeWindow("frame",920,500);
  cv::namedWindow("frame");
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
#endif

#ifdef RUN_SLAM
    api_ = std::make_unique<OpenVSLAM_API>(run_, "./config.yaml", "./orb_vocab.dbow2");
    api_->startMonoThread();
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

#ifdef RUN_SLAM
        cv::Mat greyMat;
        cv::cvtColor(mat, greyMat, cv::COLOR_BGR2GRAY);
        api_->addFrameToQueue(greyMat);
#endif
        // NOTE: In case there are some gdk/pangolin crashes
        // 1. comment out the line below and display only the frame displayed
        // with keypoints on L92 of pangolin_viewer/viewer.cc
        // 2. Comment out L92-93 of pangolin_viewer/viewer.cc and uncomment the
        // cv::waitKey(1) below
        cv::imshow("frame", mat.clone());
        // cv::waitKey(1);
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

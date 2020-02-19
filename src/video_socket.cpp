#include "video_socket.hpp"
#include "utils.hpp"

VideoSocket::VideoSocket(boost::asio::io_service& io_service,
  const std::string& drone_ip,
  const std::string& drone_port,
  const std::string& local_port
):
  BaseSocket(io_service, drone_ip, drone_port, local_port)
{
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

    boost::thread run_thread(boost::bind(&boost::asio::io_service::run, boost::ref(io_service_)));
}

void VideoSocket::handleResponseFromDrone(const boost::system::error_code& error, size_t bytes_recvd)
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

  socket_.async_receive_from(
    boost::asio::buffer(data_, max_length_),
    endpoint_,
    boost::bind(&VideoSocket::handleResponseFromDrone,
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
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
        cv::imshow("frame", mat);
        cv::waitKey(1);
      }
      next += consumed;
    }
  }
  catch (...) {
    LogInfo() << "Error";
  }
}

VideoSocket::~VideoSocket(){
  socket_.close();
}

void VideoSocket::handleSendCommand(const boost::system::error_code& error,
   size_t bytes_sent, const std::string& cmd)
{
  std::cout << "VideoSocket class does not implement handleSendCommand()" << std::endl;
}

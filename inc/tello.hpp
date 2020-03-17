#include  <memory>

#include "command_socket.hpp"
#include "video_socket.hpp"
#include "state_socket.hpp"
#include "joystick.hpp"
#include "utils.hpp"

class Tello{
public:
  Tello(
#ifdef USE_BOOST
    boost::asio::io_service& io_service,
#else
    asio::io_service& io_service,
#endif
std::condition_variable& cv_run
);
~Tello();
std::unique_ptr<CommandSocket> cs;
std::unique_ptr<Joystick> js_;
std::unique_ptr<VideoSocket> vs;
std::unique_ptr<StateSocket> ss;
private:
  #ifdef USE_BOOST
    boost::asio::io_service& io_service_;
    boost::thread js_thread_;
  #else
    asio::io_service& io_service_;
    std::thread js_thread_;
  #endif
  std::condition_variable& cv_run_;
  void jsToCommandThread();
  void jsToCommand(ButtonId update);
  void jsToCommand(AxisId update);
  bool run_ = true;
};

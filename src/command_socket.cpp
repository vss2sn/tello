#include <chrono>
#include <thread>

#include "tello/command_socket.hpp"
#include "utils/utils.hpp"

#define UDP asio::ip::udp
#define ASYNC_RECEIVE                                                          \
  socket_.async_receive_from(                                                  \
      asio::buffer(data_, max_length_), endpoint_,                             \
      [&](const std::error_code &error, size_t bytes_recvd) {                  \
        return handleResponseFromDrone(error, bytes_recvd);                    \
      }); // [&](auto... args){return handleResponseFromDrone(args...);});
#define ASYNC_SEND                                                             \
  socket_.async_send_to(                                                       \
      asio::buffer(cmd, cmd.size()), endpoint_,                                \
      [&](const std::error_code &error, size_t bytes_recvd) {                  \
        return handleSendCommand(error, bytes_recvd, cmd);                     \
      }); // [&](auto... args){return handleSendCommand(args..., cmd);});

// NOTE: Possible methods to call threads
// io_thread(boost::bind(&boost::asio::io_service::run,
// boost::ref(io_service_))); io_thread = std::thread([&]{io_service_.run();});
// io_thread = std::thread(
// static_cast<std::size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run),
// io_service); cmd_thread = std::thread(&CommandSocket::sendQueueCommands,
// this); cmd_thread = std::thread([&](void){sendQueueCommands();});

namespace constants {
constexpr auto joystick_timeout = std::chrono::milliseconds(1);
constexpr auto waiting_for_response_timeout = std::chrono::milliseconds(100);
constexpr auto minor_pause = std::chrono::seconds(1);
constexpr auto dnal_timeout = std::chrono::seconds(7);
} // namespace constants

CommandSocket::CommandSocket(asio::io_service &io_service,
                             const std::string &drone_ip,
                             const std::string &drone_port,
                             const std::string &local_port,
                             int n_retries_allowed, int timeout)
    : BaseSocket(io_service, drone_ip, drone_port, local_port),
      timeout_(timeout), n_retries_allowed_(n_retries_allowed) {
  // NOTE: Used #define UDP to handle namespace
  UDP::resolver resolver(io_service_);
  UDP::resolver::query query(UDP::v4(), drone_ip_, drone_port_);
  UDP::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;
  io_thread =
      std::thread([&] {
        io_service_.run();
        utils_log::LogDebug()
            << "----------- Command socket io_service thread exits -----------";
      });
  cmd_thread = std::thread(&CommandSocket::sendQueueCommands, this);
  dnal_thread = std::thread(&CommandSocket::doNotAutoLandWorker, this);
  io_thread.detach();
  cmd_thread.detach();
  dnal_thread.detach();
  command_sent_time_ = std::chrono::system_clock::now();
  ASYNC_RECEIVE;
}

void CommandSocket::handleResponseFromDrone(const std::error_code &error,
                                            size_t bytes_recvd) {
  if (!error && bytes_recvd > 0) {
    waiting_for_response_ = false;
    response_ = "";
    const std::string response_temp = std::string(data_);
    // remove additional random characters sent over UDP
    // TODO(vss): Make this better
    for (int i = 0; i < bytes_recvd && (isprint(response_temp[i]) != 0); ++i) {
      response_ += response_temp[i];
    }
    utils_log::LogInfo() << "Received response [" << response_
                         << "] after sending command [" << last_command_
                         << "] from address [" << drone_ip_ << ":"
                         << drone_port_ << "].";
  } else {
    utils_log::LogWarn() << "Error/Nothing received.";
  }
  ASYNC_RECEIVE;
}

void CommandSocket::sendCommand(const std::string &cmd) {
  n_retries_ = 0;
  ASYNC_SEND;
  std::this_thread::sleep_for(
      constants::joystick_timeout); // TODO(vss): reduce this to less than
                                    // amount of time joystick waits?
}

void CommandSocket::waitForResponse() {
  std::chrono::system_clock::time_point start =
      std::chrono::system_clock::now();
  while (std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now() - start)
                 .count() < timeout_ &&
         on_) {
    if (!waiting_for_response_) {
      break;
    }
    std::this_thread::sleep_for(
        constants::waiting_for_response_timeout); // TODO(vss): replace with
                                                  // condition variable
  }
  if (!on_) {
    waiting_for_response_ = false;
    return;
  }
  if (waiting_for_response_) {
    utils_log::LogInfo() << "Timeout - Attempt #" << n_retries_
                         << " for command [" << last_command_ << "].";
  }
  if (n_retries_ == n_retries_allowed_) {
    if (n_retries_allowed_ > 0) {
      utils_log::LogWarn() << "Exhausted retries.";
    }
    waiting_for_response_ = false; // Timeout
  }
}

void CommandSocket::handleSendCommand(const std::error_code &error,
                                      size_t bytes_sent, std::string cmd) {
  if (!error && bytes_sent > 0) {
    utils_log::LogInfo() << "Successfully sent command [" << cmd
                         << "] to address [" << drone_ip_ << ":" << drone_port_
                         << "].";
    last_command_ = cmd;
    command_sent_time_ = std::chrono::system_clock::now();
  } else {
    utils_log::LogDebug() << "Failed to send command [" << cmd << "].";
  }
}

void CommandSocket::retry(const std::string &cmd) {
  last_command_ = cmd;
  waitForResponse();
  if (!waiting_for_response_) {
    return;
  }
  while (n_retries_ < n_retries_allowed_ && waiting_for_response_ && on_) {
    utils_log::LogInfo() << "Retrying...";
    n_retries_++;
    ASYNC_SEND;
    waitForResponse();
  }
}

void CommandSocket::addCommandToQueue(const std::string &cmd) {
  queue_mutex_.lock();
  utils_log::LogInfo() << "Added command [" << cmd << "] to queue.";
  command_queue_.push_back(cmd);
  queue_mutex_.unlock();
  cv_execute_queue_.notify_all();
}

void CommandSocket::executeQueue() {
  utils_log::LogInfo() << "Executing queue commands.";
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = true;
  }
  cv_execute_queue_.notify_all();
}

void CommandSocket::sendQueueCommands() {
  std::string cmd;
  while (on_) {
    std::this_thread::sleep_for(constants::minor_pause);
    // usleep(1000); // allow others to grab lock
    {
      std::unique_lock<std::mutex> lk(m);
      cv_execute_queue_.wait(lk, [this] {
        return (execute_queue_ && !command_queue_.empty()) || !on_;
      });
    }
    if (!on_) {
      break;
    }
    while (!command_queue_.empty() && execute_queue_) {
      queue_mutex_.lock();
      cmd = command_queue_.front();
      command_queue_.pop_front();
      queue_mutex_.unlock();
      if (cmd.substr(0, 5) == "delay") {
        std::this_thread::sleep_for(
            std::chrono::seconds(stoi(cmd.substr(5, cmd.size()))));
        // usleep(1000000 * stoi(cmd.substr(5, cmd.size())));
        continue;
      } else if ((cmd.compare(0, 4, "stop") == 0) ||
                 (cmd.compare(0, 9, "emergency") == 0)) {
        waiting_for_response_ =
            false; // to prevent retries of prev sent command if none received
                   // in spit of comman dbeing sent.
      }
      // Run reponse thread only after sending command rather than always
      // TODO(vss): Use condition variable for the wait for response
      if (waiting_for_response_) {
        // usleep(10000); // Next send command grabs mutex before
        // handleSendCommand.
        // TODO(vss): The above should no longer be required
        utils_log::LogInfo()
            << "Waiting to send command [" << cmd
            << "] as no response has been received for the previous command. "
               "Thread calling send command paused.";
      }
      while (waiting_for_response_) {
        if (!on_) {
          break;
        }
        std::this_thread::sleep_for(constants::waiting_for_response_timeout);
      }
      waiting_for_response_ = true;
      sendCommand(cmd);
      // NOTE: Do not comment. Set n_retries_allowed_ to 0 if required.
      // If the commmand is rc, do not retry/wait for a response
      if (cmd.substr(0, 2) != "rc") {
        std::thread run_thread([&] { retry(cmd); });
        run_thread.detach();
      }
    }
  }
  utils_log::LogDebug()
      << "----------- Send queue commands thread exits -----------";
}

void CommandSocket::addCommandToFrontOfQueue(const std::string &cmd) {
  queue_mutex_.lock();
  command_queue_.push_front(cmd);
  queue_mutex_.unlock();
}

void CommandSocket::stopQueueExecution() {
  utils_log::LogInfo() << "Stopping queue execution. " << command_queue_.size()
                       << " commands still in queue.";
  std::lock_guard<std::mutex> lk(m);
  execute_queue_ = false;
}

void CommandSocket::clearQueue() {
  utils_log::LogInfo() << "Clearing queue.";
  queue_mutex_.lock();
  command_queue_.clear();
  queue_mutex_.unlock();
}

void CommandSocket::removeNextFromQueue() {
  queue_mutex_.lock();
  std::string cmd = command_queue_.front();
  command_queue_.pop_front();
  queue_mutex_.unlock();
  utils_log::LogInfo() << "Removed command [" << cmd << "] from queue.";
}

void CommandSocket::doNotAutoLand() {
  utils_log::LogDebug() << "Automatic landing disabled.";
  {
    std::lock_guard<std::mutex> lk(dnal_mutex);
    dnal_ = true;
  }
  cv_dnal_.notify_all();
}

void CommandSocket::allowAutoLand() {
  utils_log::LogDebug() << "Automatic landing enabled.";
  {
    std::lock_guard<std::mutex> lk(dnal_mutex);
    dnal_ = false;
  }
  cv_dnal_.notify_all();
}

void CommandSocket::doNotAutoLandWorker() {
  std::chrono::system_clock::time_point start;
  std::string cmd = "rc 0 0 0 0";
  sendCommand(cmd);
  while (on_) {
    {
      std::unique_lock<std::mutex> lk(dnal_mutex);
      cv_dnal_.wait(lk, [this] {
        return (((!execute_queue_) ||
                 (execute_queue_ && command_queue_.empty())) &&
                dnal_) ||
               !on_;
      });
    }
    if (!on_) {
      break;
    }
    if (std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now() - command_sent_time_) >
        constants::dnal_timeout) {
      sendCommand(cmd);
      last_command_ = cmd;
      waiting_for_response_ = false;
    } else {
      std::this_thread::sleep_for(
          constants::dnal_timeout); // 10 seconds, command needs to be sent ever
                                    // 15
    }
  }
  utils_log::LogDebug() << "----------- DNAL worker thread exits -----------";
}

void CommandSocket::stop() {
  std::lock_guard<std::mutex> lk(m);
  execute_queue_ = false;
  sendCommand("stop");
}

void CommandSocket::emergency() {
  std::lock_guard<std::mutex> lk(m);
  execute_queue_ = false;
  sendCommand("emergency");
}

bool CommandSocket::isExecutingQueue() const { return execute_queue_; }

void CommandSocket::land() {
  allowAutoLand();
  sendCommand("land");
}

// TODO(vss): Refactor exit of dnal worker thread
CommandSocket::~CommandSocket() {
  {
    std::lock_guard<std::mutex> lk(dnal_mutex);
    dnal_ = true; // allow dnal thread to exit
    std::lock_guard<std::mutex> lk_2(m);
    execute_queue_ = false; // also allow dnal thread to exit
    on_ = false;
  }
  cv_dnal_.notify_all();
  std::this_thread::sleep_for(
      constants::minor_pause); // allow dnal thread to exit before setting
                               // execute_queue_ to true.
  {
    std::lock_guard<std::mutex> lk(m);
    execute_queue_ = true;
  }
  cv_execute_queue_.notify_all();
}

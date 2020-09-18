#include "tello/command_socket.hpp"

#include <chrono>
#include <thread>

#include "utils/utils.hpp"

#define UDP asio::ip::udp
#define ASYNC_RECEIVE                                         \
  socket_.async_receive_from(                                 \
      asio::buffer(data_, max_length_), endpoint_,            \
      [&](const std::error_code &error, size_t bytes_recvd) { \
        return handleResponseFromDrone(error, bytes_recvd);   \
      });  // [&](auto... args){return handleResponseFromDrone(args...);});
#define ASYNC_SEND                                            \
  socket_.async_send_to(                                      \
      asio::buffer(cmd, cmd.size()), endpoint_,               \
      [&](const std::error_code &error, size_t bytes_recvd) { \
        return handleSendCommand(error, bytes_recvd, cmd);    \
      });  // [&](auto... args){return handleSendCommand(args..., cmd);});

// NOTE: Possible methods to call threads
// io_thread_(boost::bind(&boost::asio::io_service::run,
// boost::ref(io_service_))); io_thread_ = std::thread([&]{io_service_.run();});
// io_thread_ = std::thread(
// static_cast<std::size_t(boost::asio::io_service::*)()>(&boost::asio::io_service::run),
// io_service); cmd_thread = std::thread(&CommandSocket::sendQueueCommands,
// this); cmd_thread = std::thread([&](void){sendQueueCommands();});

namespace constants {
constexpr auto joystick_timeout = std::chrono::milliseconds(1);
constexpr auto waiting_for_response_timeout = std::chrono::milliseconds(100);
constexpr auto minor_pause = std::chrono::seconds(1);
constexpr auto dnal_timeout = std::chrono::seconds(7);
}  // namespace constants

inline bool timedOut(const std::chrono::system_clock::time_point &start,
                     int timeout) {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::system_clock::now() - start)
             .count() >= timeout;
}

CommandSocket::CommandSocket(asio::io_service &io_service,
                             const std::string &drone_ip,
                             const std::string &drone_port,
                             const std::string &local_port,
                             int n_retries_allowed, int timeout)
    : BaseSocket(io_service, drone_ip, drone_port, local_port),
      timeout_(timeout),
      n_retries_allowed_(n_retries_allowed) {
  // NOTE: Used #define UDP to handle namespace
  UDP::resolver resolver(io_service_);
  UDP::resolver::query query(UDP::v4(), drone_ip_, drone_port_);
  UDP::resolver::iterator iter = resolver.resolve(query);
  endpoint_ = *iter;
  ASYNC_RECEIVE;
  io_thread_ =
      std::thread([&, this] {
        io_service_.run();
        utils_log::LogDebug()
            << "----------- Command socket io_service thread exits -----------";
      });
  cmd_thread = std::thread(&CommandSocket::sendQueueCommands, this);
  dnal_thread = std::thread(&CommandSocket::doNotAutoLandWorker, this);
  command_sent_time_ = std::chrono::system_clock::now();
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
  std::this_thread::sleep_for(constants::joystick_timeout);
  // TODO(vss): reduce this to less than amount of time joystick waits?
}

void CommandSocket::waitForResponse() {
  while (!timedOut(command_sent_time_, timeout_) && on_ &&
         waiting_for_response_) {
    std::this_thread::sleep_for(constants::waiting_for_response_timeout);
  }
  if (!on_) {
    waiting_for_response_ = false;
    utils_log::LogDebug() << "Exiting waitForResponse()";
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
    waiting_for_response_ = false;
  }
  if (response_ == "error Not joystick") {
    utils_log::LogInfo() << "Received `error Not joystick`. Will retry sending and wait for response.";
    --n_retries_;
    waiting_for_response_ = true;
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

void CommandSocket::addCommandToQueue(const std::string &cmd) {
  {
    std::lock_guard<std::mutex> l(queue_mutex_);
    utils_log::LogInfo() << "Added command [" << cmd << "] to queue.";
    command_queue_.push_back(cmd);
  }
  cv_execute_queue_.notify_all();
}

void CommandSocket::executeQueue() {
  utils_log::LogInfo() << "Executing queue commands.";
  {
    std::lock_guard<std::mutex> l(execute_queue_mutex_);
    execute_queue_ = true;
  }
  cv_execute_queue_.notify_all();
}

bool CommandSocket::shouldWaitForResponse(const std::string &cmd) {
  if (cmd.compare(0, 2, "rc") == 0) {
    return false;
  }
  std::string new_cmd;
  if (!command_queue_.empty()) {
    {
      std::lock_guard<std::mutex> l(queue_mutex_);
      new_cmd = command_queue_.front();
    }
    if (cmd.compare(0, 4, "stop") == 0 || cmd.compare(0, 9, "emergency") == 0 ||
        cmd.compare(0, 5, "delay") == 0) {
      return false;
    }
  }
  return true;
}

void CommandSocket::sendQueueCommands() {
  std::string cmd;
  while (on_) {
    // allow others to grab lock
    std::this_thread::sleep_for(constants::minor_pause);
    while (!command_queue_.empty() && execute_queue_ && on_) {
      {
        std::lock_guard<std::mutex> l(queue_mutex_);
        cmd = command_queue_.front();
        command_queue_.pop_front();
        // In case of a typo, though sending out the incorrect command
        // will receive an error response from the drone anyway
        while (cmd == "") {
          cmd = command_queue_.front();
          command_queue_.pop_front();
        }
      }
      if (cmd.substr(0, 5) == "delay") {
        std::this_thread::sleep_for(
            std::chrono::seconds(stoi(cmd.substr(5, cmd.size()))));
        continue;
      }
      waiting_for_response_ = true;
      sendCommand(cmd);
      // TODO: modify this so each command has its own options (user specified)
      if (shouldWaitForResponse(cmd)) {
        last_command_ = cmd;
        n_retries_ = 0;
        utils_log::LogInfo() << "Waiting for response for [" << cmd << "] ";
        waitForResponse();
        while (n_retries_ < n_retries_allowed_ && waiting_for_response_ &&
               on_) {
          utils_log::LogInfo() << "Retrying...";
          n_retries_++;
          ASYNC_SEND;
          waitForResponse();
        }
      } else {
        waiting_for_response_ = false;
      }
    }
    {
      std::unique_lock<std::mutex> lk(execute_queue_mutex_);
      cv_execute_queue_.wait(lk, [this] {
        return (execute_queue_ && !command_queue_.empty()) || !on_;
      });
    }
    allowAutoLand();
  }
  utils_log::LogDebug()
      << "----------- Send queue commands thread exits -----------";
}

void CommandSocket::addCommandToFrontOfQueue(const std::string &cmd) {
  std::lock_guard<std::mutex> l(queue_mutex_);
  command_queue_.push_front(cmd);
}

void CommandSocket::stopQueueExecution() {
  std::lock_guard<std::mutex> l(execute_queue_mutex_);
  utils_log::LogInfo() << "Stopping queue execution. " << command_queue_.size()
                       << " commands still in queue.";
  execute_queue_ = false;
}

void CommandSocket::clearQueue() {
  utils_log::LogInfo() << "Clearing queue.";
  std::lock_guard<std::mutex> l(queue_mutex_);
  command_queue_.clear();
}

void CommandSocket::removeNextFromQueue() {
  {
    std::lock_guard<std::mutex> l(queue_mutex_);
    std::string cmd = command_queue_.front();
    utils_log::LogInfo() << "Removed command [" << cmd << "] from queue.";
    command_queue_.pop_front();
  }
}

void CommandSocket::doNotAutoLand() {
  utils_log::LogDebug() << "Automatic landing disabled.";
  {
    std::lock_guard<std::mutex> lk(dnal_mutex_);
    dnal_ = true;
  }
  cv_dnal_.notify_all();
}

void CommandSocket::allowAutoLand() {
  utils_log::LogDebug() << "Automatic landing enabled.";
  {
    std::lock_guard<std::mutex> lk(dnal_mutex_);
    dnal_ = false;
  }
  cv_dnal_.notify_all();
}

void CommandSocket::doNotAutoLandWorker() {
  std::chrono::system_clock::time_point start;
  std::string cmd = "rc 0 0 0 0";
  while (on_) {
    // NOTE: command needs to be sent within 15 seconds of previous command
    if (timedOut(command_sent_time_, constants::dnal_timeout.count())) {
      sendCommand(cmd);
      last_command_ = cmd;
      waiting_for_response_ = false;
    } else {
      utils_log::LogDebug()
          << "----------- DNAL worker thread sleeps -----------";
      std::this_thread::sleep_for(constants::dnal_timeout);
      utils_log::LogDebug()
          << "----------- DNAL worker thread wakes up -----------";
    }
    {
      std::unique_lock<std::mutex> lk(dnal_mutex_);
      cv_dnal_.wait(lk, [this] {
        return !on_ || (dnal_ && (!execute_queue_ ||
                                  (execute_queue_ && command_queue_.empty())));
        // exit code or dnal && is queue executing or was executing and now
        // empty
      });
    }
  }
  utils_log::LogDebug() << "----------- DNAL worker thread exits -----------";
}

void CommandSocket::stop() {
  std::lock_guard<std::mutex> l(execute_queue_mutex_);
  execute_queue_ = false;
  sendCommand("stop");
}

void CommandSocket::emergency() {
  std::lock_guard<std::mutex> l(execute_queue_mutex_);
  execute_queue_ = false;
  sendCommand("emergency");
}

bool CommandSocket::isExecutingQueue() const { return execute_queue_; }

void CommandSocket::land() {
  allowAutoLand();
  sendCommand("land");
}

void CommandSocket::prepareToExit() {
  stop();  // stop queue execution
  land();  // land
  utils_log::LogWarn() << "----------- Landing -----------";
  utils_log::LogWarn()
      << "----------- preparing to exit command socket -----------";
  {
    // This might take some time
    std::scoped_lock(execute_queue_mutex_, dnal_mutex_);
    on_ = false;  // start exitting threads
  }
  cv_dnal_.notify_all();
  if (dnal_thread.joinable()) {
    dnal_thread.join();
  }
  cv_execute_queue_.notify_all();
  if (cmd_thread.joinable()) {
    cmd_thread.join();
  }
  utils_log::LogWarn()
      << "----------- end of preparing to exit command socket -----------";
}

CommandSocket::~CommandSocket() {
  utils_log::LogWarn() << "----------- Command socket destructor -----------";
}

bool CommandSocket::dnal() const { return dnal_; }

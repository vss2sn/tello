#ifdef USE_CONFIG

#ifndef CONFIG_HANDLER_HPP
#define CONFIG_HANDLER_HPP

#include <condition_variable>
#include <map>

#include "asio.hpp"
#include "tello.hpp"

std::map<std::string, std::unique_ptr<Tello>> handleConfig(
  const std::string& config_file,
  asio::io_service& io_service,
  std::condition_variable& cv_run
);

#endif // CONFIG_HANDLER_HPP
#endif // USE_CONFIG

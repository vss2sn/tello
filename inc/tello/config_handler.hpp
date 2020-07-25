#ifdef USE_CONFIG

#ifndef CONFIG_HANDLER_HPP
#define CONFIG_HANDLER_HPP

#include <condition_variable>
#include <map>

#include "asio.hpp"
#include "tello/tello.hpp"

/**
* @brief Function to create a tello class object using a configuraion file
* @param [in] config_file Path to configuration file
* @param [in] io_service io_service object
* @param [in] cv_run conition variable that is notified when the code needs to exit
* @return <return_description>
* @details <details>
*/
std::map<std::string, std::unique_ptr<Tello>> handleConfig(
  const std::string& config_file,
  asio::io_service& io_service,
  std::condition_variable& cv_run
);

#endif // CONFIG_HANDLER_HPP
#endif // USE_CONFIG

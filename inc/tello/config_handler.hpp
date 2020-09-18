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
 * @return <return_description>
 * @details <details>
 */
std::map<std::string, std::unique_ptr<Tello>> handleConfig(const std::string &config_file);

#endif // CONFIG_HANDLER_HPP
#endif // USE_CONFIG

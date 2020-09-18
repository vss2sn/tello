#ifdef USE_CONFIG

#include "tello/config_handler.hpp"

#include <yaml-cpp/yaml.h>

#include <map>

#include "tello/tello.hpp"
#include "utils/utils.hpp"

std::map<std::string, std::unique_ptr<Tello>> handleConfig(
    const std::string &config_file) {
  utils_log::LogInfo() << "Loading config file.";
  std::map<std::string, std::unique_ptr<Tello>> m;
  YAML::Node config = YAML::LoadFile(config_file);
  const int n_groups = config["groups"].as<int>();
  if (n_groups == 0) {
    utils_log::LogErr()
        << "No groups defined. Please check config file. Exiting";
    exit(0);
  }

  const std::string group = "group";
  const std::string type = "type";

  for (auto group_n = 0; group_n < n_groups; group_n++) {
    const std::string group_number = group + std::to_string(group_n);
    if (!config[group_number]) {
      utils_log::LogWarn()
          << "Group " << group_n
          << " not defined. Please check config file. Proceeding.";
      continue;
    }
    const int n_types = config[group_number]["types"].as<int>();

    for (auto n_type = 0; n_type < n_types; n_type++) {
      const std::string type_number = type + std::to_string(n_type);
      if (!config[group_number][type_number]) {
        utils_log::LogWarn()
            << "Type " << n_type
            << " not defined. Please check config file. Proceeding.";
        continue;
      }
      std::string type_id =
          config[group_number][type_number]["type_id"].as<std::string>();
      int n_members = config[group_number][type_number]["members"].as<int>();

      for (auto member_n = 0; member_n < n_members; member_n++) {
        std::string identifier = std::to_string(group_n) + "." + type_id + "." +
                                 std::to_string(member_n);

        auto a = std::make_unique<Tello>(
            config[type_id]["drone_ip"].as<std::string>(),
            config[type_id]["drone_port"].as<std::string>(),
            config[type_id]["video_port"].as<std::string>(),
            config[type_id]["state_port"].as<std::string>(),
            config[type_id]["camera_config_file"].as<std::string>(),
            config[type_id]["vocabulary_file"].as<std::string>(),
            config[type_id]["retries"].as<int>(),
            config[type_id]["timeout"].as<int>(),
            config[type_id]["load_map_db_path"].as<std::string>(),
            config[type_id]["save_map_db_path"].as<std::string>(),
            config[type_id]["mask_img_path"].as<std::string>(),
            config[type_id]["load_map"].as<bool>(),
            config[type_id]["continue_mapping"].as<bool>(),
            config[type_id]["scale"]
                .as<double>(),  // NOTE: double to float implicit conversion
            config[type_id]["sequence_file"].as<std::string>()
            // TODO: Config object?
        );
        m.insert(std::pair<std::string, std::unique_ptr<Tello>>(identifier,
                                                                std::move(a)));
      }
    }
  }
  utils_log::LogInfo() << "Config file loaded and parsed.";
  return m;
}

struct ID {
  int group_n, member_n;
  std::string type_id;
};

#endif  // USE_CONFIG

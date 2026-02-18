/*
 *   Copyright 2026 Huawei Technologies Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include <tracr/marker_management_engine.hpp>

namespace fs = std::filesystem;

/**
 *
 */
// In your .cpp file, above the function
constexpr const char PARAVER_HEADER[] = "DEFAULT_OPTIONS\n\n"
                                        "LEVEL               THREAD\n"
                                        "UNITS               NANOSEC\n"
                                        "LOOK_BACK           100\n"
                                        "SPEED               1\n"
                                        "FLAG_ICONS          ENABLED\n"
                                        "NUM_OF_STATE_COLORS 1000\n"
                                        "YMAX_SCALE          37\n\n"

                                        "DEFAULT_SEMANTIC\n\n"
                                        "THREAD_FUNC         State As Is\n\n"

                                        "STATES_COLOR\n"
                                        "0   {  0,   0,   0}\n"
                                        "1   {  0, 130, 200}\n"
                                        "2   {217, 217, 217}\n"
                                        "3   {230,  25,  75}\n"
                                        "4   { 60, 180,  75}\n"
                                        "5   {255, 225,  25}\n"
                                        "6   {245, 130,  48}\n"
                                        "7   {145,  30, 180}\n"
                                        "8   { 70, 240, 240}\n"
                                        "9   {240,  50, 230}\n"
                                        "10  {210, 245,  60}\n"
                                        "11  {250, 190, 212}\n"
                                        "12  {  0, 128, 128}\n"
                                        "13  {128, 128, 128}\n"
                                        "14  {220, 190, 255}\n"
                                        "15  {170, 110,  40}\n"
                                        "16  {255, 250, 200}\n"
                                        "17  {128,   0,   0}\n"
                                        "18  {170, 255, 195}\n"
                                        "19  {128, 128,   0}\n"
                                        "20  {255, 215, 180}\n"
                                        "21  {  0,   0, 128}\n"
                                        "22  {  0,   0, 255}\n\n"

                                        "EVENT_TYPE\n"
                                        "0 90         TraCR\n"
                                        "VALUES\n";

/**
 *
 */
static const char *perfetto_colors[] = {"yellow", "olive", "purple", "blue",
                                        "green",  "red",   "pink"};

/**
 * A function to load a bts file into a std::vector<Payload>
 */
bool load_bts_file(const fs::path &filepath,
                   std::vector<TraCR::Payload> &traces, size_t &out_count) {
  std::ifstream ifs(filepath, std::ios::binary);
  if (!ifs) {
    std::cerr << "Failed to open file: " << filepath << "\n";
    return false;
  }

  // Determine file size
  ifs.seekg(0, std::ios::end);
  std::streamsize filesize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  size_t count = filesize / sizeof(TraCR::Payload);

  // Resize the vector to hold the data
  traces.resize(count);

  ifs.read(reinterpret_cast<char *>(traces.data()),
           count * sizeof(TraCR::Payload));
  if (!ifs) {
    std::cerr << "Failed to read all data from file: " << filepath << "\n";
    return false;
  }

  out_count = count;
  return true;
}

/**
 * A function for extracting the optional json information
 */
int get_extra_info(nlohmann::json &extra_info, char *argv3) {
  // Load the json metadata file
  fs::path json_file = argv3;
  if (fs::exists(json_file)) {
    std::ifstream ifs(json_file);
    if (ifs.is_open()) {
      try {
        ifs >> extra_info;
        std::cout << "  Loaded custom JSON:\n" << extra_info.dump(4) << "\n";
      } catch (const std::exception &e) {
        std::cerr << "  Failed to parse JSON: " << e.what() << "\n";
        return 1;
      }
    } else {
      std::cerr << "  Failed to open: " << argv3 << "\n";
      return 1;
    }
  } else {
    std::cerr << "  No '" << argv3 << "' found\n";
    return 1;
  }
  return 0;
}

/**
 * Goes through all the thread folder and loads all the bts files from
 * the given proc folder
 */
int load_thread_traces(const fs::path &proc_path,
                       std::vector<std::vector<TraCR::Payload>> &bts_files,
                       std::vector<pid_t> &bts_tids) {
  for (const auto &thread_entry : fs::directory_iterator(proc_path)) {

    if (!thread_entry.is_directory())
      continue;

    const std::string folder_name = thread_entry.path().filename().string();

    if (folder_name.find("thread.") != 0)
      continue;

    fs::path trace_file = thread_entry.path() / "traces.bts";

    if (!fs::exists(trace_file)) {
      std::cerr << "  No trace file in: " << thread_entry.path() << "\n";
      return 1;
    }

    std::cout << "  Found trace file: " << trace_file << "\n";

    // Load traces
    std::vector<TraCR::Payload> traces;
    size_t num_traces = 0;

    if (!load_bts_file(trace_file, traces, num_traces)) {
      std::cerr << "  Failed to load bts file: " << trace_file << "\n";
      return 1;
    }

    std::cout << "Loaded " << num_traces << " traces from " << trace_file
              << "\n";

    // Extract TID
    std::size_t dot_pos = folder_name.find('.');
    if (dot_pos == std::string::npos) {
      std::cerr << "  Invalid thread folder name: " << folder_name << "\n";
      return 1;
    }

    pid_t tid;
    try {
      tid = std::stoi(folder_name.substr(dot_pos + 1));
    } catch (const std::exception &) {
      std::cerr << "  Error parsing TID in folder: " << folder_name << "\n";
      return 1;
    }

    bts_files.push_back(std::move(traces));
    bts_tids.push_back(tid);
  }

  return 0;
}

/**
 *
 */
int load_metadata_json(const fs::path &proc_path, nlohmann::json &metadata) {
  fs::path json_file = proc_path / "metadata.json";

  if (!fs::exists(json_file)) {
    std::cerr << "  No metadata.json found\n";
    return 1;
  }

  std::ifstream ifs(json_file);
  if (!ifs.is_open()) {
    std::cerr << "  Failed to open metadata.json\n";
    return 1;
  }

  try {
    ifs >> metadata;
    std::cout << "  Loaded metadata.json:\n" << metadata.dump(4) << "\n";
  } catch (const std::exception &e) {
    std::cerr << "  Failed to parse JSON: " << e.what() << "\n";
    return 1;
  }

  return 0;
}

/**
 * A function for extracting the bts and metadata
 */
int extract_bts_metadata(std::vector<std::vector<TraCR::Payload>> &bts_files,
                         std::vector<pid_t> &bts_tids, nlohmann::json &metadata,
                         const fs::path base_path, int &pid) {

  bool proc_folder_found = false;
  for (const auto &proc_entry : fs::directory_iterator(base_path)) {
    if (proc_entry.is_directory() &&
        proc_entry.path().filename().string().find("proc.") == 0) {
      std::cout << "Found proc folder: " << proc_entry.path() << "\n";

      if (proc_folder_found) {
        std::cerr << "Error: Currently, having more than one proc folder is "
                     "illegal.\n";
        return 1;
      }
      proc_folder_found = true;

      // Store the PID, might be helpful for later
      std::string folder_name = proc_entry.path().filename().string();
      std::size_t dot_pos = folder_name.find('.');
      if (dot_pos != std::string::npos) {
        std::string pid_str = folder_name.substr(dot_pos + 1);
        try {
          pid = std::stoi(pid_str);
        } catch (const std::exception &e) {
          std::cerr << "  Error parsing TID in folder: " << folder_name << "\n";
          return 1;
        }
      }

      // Load the json metadata file
      if (load_metadata_json(proc_entry.path(), metadata) != 0) {
        return 1;
      }

      // Iterate over thread folders inside proc.*
      if (load_thread_traces(proc_entry.path(), bts_files, bts_tids) != 0) {
        std::cerr << "Error: load_thread_traces() failed.\n";
        return 1;
      }
    }
  }

  if (!proc_folder_found) {
    std::cerr << "Error: No proc folder found.\n";
    return 1;
  }

  return 0;
}

/**
 * Finds the next payload with the smallest timestamp across all BTS files
 * Returns the index of the file containing it
 */
size_t
find_next_payload(const std::vector<std::vector<TraCR::Payload>> &bts_files,
                  const std::vector<size_t> &bts_files_ptrs,
                  TraCR::Payload &out_payload) {
  uint64_t next_timestamp = std::numeric_limits<uint64_t>::max();
  size_t index = 0;

  for (size_t i = 0; i < bts_files.size(); ++i) {
    if (bts_files_ptrs[i] < bts_files[i].size() &&
        bts_files[i][bts_files_ptrs[i]].timestamp < next_timestamp) {
      next_timestamp = bts_files[i][bts_files_ptrs[i]].timestamp;
      index = i;
    }
  }

  out_payload = bts_files[index][bts_files_ptrs[index]];
  return index;
}

/**
 * Returns true if all pointers reached the end, false otherwise
 */
bool advance_ptrs_and_check_end(
    std::vector<size_t> &bts_files_ptrs,
    const std::vector<std::vector<TraCR::Payload>> &bts_files, size_t index) {
  // Increase the pointer for the given index if not at the end
  if (bts_files_ptrs[index] < bts_files[index].size()) {
    ++bts_files_ptrs[index];
  }

  // Check if all pointers reached the limit
  for (size_t i = 0; i < bts_files.size(); ++i) {
    if (bts_files_ptrs[i] != bts_files[i].size()) {
      return false; // Not all reached the end
    }
  }
  return true; // All reached the end
}

/**
 * Store the state.cfg in the given tracr folder
 */
int copy_state_cfg(const fs::path &base_path) {
  fs::path source = "state.cfg";

  try {
    fs::copy_file(source, base_path / source.filename(),
                  fs::copy_options::overwrite_existing);

    std::cout << "File 'state.cfg' copied successfully.\n";
  } catch (const fs::filesystem_error &e) {
    std::cerr << "Error copying file: " << e.what() << '\n';
    return 1;
  }

  return 0;
}

/**
 * Create the tracr.pcf file
 */
int create_tracr_pcf(const fs::path &base_path,
                     const nlohmann::json &extra_info,
                     const nlohmann::json &metadata) {
  std::ofstream out(base_path / "tracr.pcf");
  if (!out) {
    std::cerr << "Error opening tracr.pcf for writing\n";
    return 1;
  }

  // Fixed Paraver stuff
  out << PARAVER_HEADER;

  // Write markerTypes as VALUES
  const nlohmann::json *markerTypes_json = nullptr;

  if (extra_info.contains("markerTypes") &&
      !extra_info["markerTypes"].is_null()) {
    markerTypes_json = &extra_info["markerTypes"];
  } else if (metadata.contains("markerTypes") &&
             !metadata["markerTypes"].is_null()) {
    markerTypes_json = &metadata["markerTypes"];
  }

  if (markerTypes_json) {
    for (auto it = markerTypes_json->begin(); it != markerTypes_json->end();
         ++it) {
      out << it.key() << "   " << it.value() << "\n";
    }
  }

  out.close();
  std::cout << "tracr.pcf written successfully.\n";

  return 0;
}

/**
 *
 */
void extract_channel_info(const nlohmann::json &extra_info,
                          const nlohmann::json &metadata, size_t &num_channels,
                          std::stringstream &ss) {
  num_channels = 1; // default

  if (extra_info.contains("channel_names") &&
      !extra_info["channel_names"].is_null()) {

    num_channels = extra_info["channel_names"].size();
    for (auto &channel_name : extra_info["channel_names"])
      ss << channel_name << "\n";

  } else if (metadata.contains("channel_names") &&
             !metadata["channel_names"].is_null()) {

    num_channels = metadata["channel_names"].size();
    for (auto &channel_name : metadata["channel_names"])
      ss << channel_name << "\n";

  } else if (metadata.contains("num_channels") &&
             !metadata["num_channels"].is_null()) {

    num_channels = metadata["num_channels"];
  }
}

/**
 *
 */
int create_tracr_prv(const fs::path &base_path,
                     const nlohmann::json &extra_info,
                     const nlohmann::json &metadata,
                     const std::vector<std::vector<TraCR::Payload>> &bts_files,
                     size_t &num_channels, std::stringstream &ss) {
  std::ofstream out(base_path / "tracr.prv");
  if (!out) {
    std::cerr << "Error opening tracr.prv for writing\n";
    return 1;
  }

  // Determine channel names / number of channels
  extract_channel_info(extra_info, metadata, num_channels, ss);

  // ---- Write Paraver header ----
  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::tm *local_tm = std::localtime(&now_time);

  out << "#Paraver (" << std::setw(2) << std::setfill('0') << local_tm->tm_mday
      << "/" << std::setw(2) << std::setfill('0') << (local_tm->tm_mon + 1)
      << "/" << std::setw(2) << std::setfill('0') << (local_tm->tm_year % 100)
      << " at " << std::setw(2) << std::setfill('0') << local_tm->tm_hour << ":"
      << std::setw(2) << std::setfill('0') << local_tm->tm_min
      << "):00000000000000000000_ns:0:1:1(" << num_channels << ":1)\n";

  // ---- Extract markerTypes keys ----
  std::vector<std::string> markerTypes_keys;

  if (extra_info.contains("markerTypes") &&
      !extra_info["markerTypes"].is_null()) {

    for (auto &[key, value] : extra_info["markerTypes"].items())
      markerTypes_keys.push_back(key);

  } else if (metadata.contains("markerTypes") &&
             !metadata["markerTypes"].is_null()) {

    for (auto &[key, value] : metadata["markerTypes"].items())
      markerTypes_keys.push_back(key);
  }

  // ---- Merge and write payloads ----
  bool first = true;
  uint64_t start_time = uint64_t(metadata["start_time"]);
  std::vector<size_t> bts_files_ptrs(bts_files.size(), 0);

  bool ptrs_end = false;

  while (!ptrs_end) {

    TraCR::Payload payload;
    size_t index = find_next_payload(bts_files, bts_files_ptrs, payload);

    if (first) {
      first = false;
      start_time = payload.timestamp;
    }

    std::string colorId;

    if (!markerTypes_keys.empty()) {
      colorId = (payload.eventId == UINT16_MAX)
                    ? "0"
                    : markerTypes_keys[payload.eventId];
    } else {
      colorId = (payload.eventId == UINT16_MAX)
                    ? "0"
                    : std::to_string(payload.eventId);
    }

    out << "2:0:1:1:" << payload.channelId + 1 << ":"
        << (payload.timestamp - start_time) << ":90:" << colorId << "\n";

    ptrs_end = advance_ptrs_and_check_end(bts_files_ptrs, bts_files, index);
  }

  out.close();
  std::cout << "tracr.prv written successfully.\n";

  return 0;
}

/**
 *
 */
int create_tracr_row(const fs::path &base_path, size_t num_channels,
                     const std::stringstream &ss) {
  std::ofstream out(base_path / "tracr.row");
  if (!out) {
    std::cerr << "Error opening tracr.row for writing\n";
    return 1;
  }

  // Fixed Paraver stuff
  out << "LEVEL NODE SIZE 1\n"
         "hostname\n\n"
         "LEVEL THREAD SIZE "
      << num_channels << "\n";

  if (!ss.str().empty()) {
    out << ss.str();
  } else {
    for (size_t i = 0; i < num_channels; ++i) {
      out << "Channel_" << i << "\n";
    }
  }

  out.close();
  std::cout << "tracr.row written successfully.\n";

  return 0;
}

/**
 * Store in Paraver format
 */
int paraver(const std::vector<std::vector<TraCR::Payload>> &bts_files,
            const std::vector<pid_t> &bts_tids, nlohmann::json &extra_info,
            nlohmann::json &metadata, const fs::path base_path, int &pid) {
  /**
   * Store the state.cfg in the given tracr folder
   */
  if (copy_state_cfg(base_path) != 0) {
    return 1;
  }

  /**
   * Create the tracr.pcf file
   */
  if (create_tracr_pcf(base_path, extra_info, metadata) != 0) {
    return 1;
  }

  /**
   * Create the tracr.prv file
   */
  size_t num_channels = 1;
  std::stringstream ss;
  if (create_tracr_prv(base_path, extra_info, metadata, bts_files, num_channels,
                       ss) != 0) {
    return 1;
  }

  /**
   * Create the tracr.row file
   */
  if (create_tracr_row(base_path, num_channels, ss) != 0) {
    return 1;
  }

  return 0;
}

/**
 *
 */
uint32_t
populate_perfetto_channels(const nlohmann::json &extra_info,
                           const nlohmann::json &metadata, uint32_t pid,
                           nlohmann::json &perfetto,
                           std::vector<std::string> &markerTypes_values) {
  uint32_t num_channels = 1; // default
  const nlohmann::json *channels_json = nullptr;

  // Determine which JSON array to use for channel names
  if (extra_info.contains("channel_names") &&
      !extra_info["channel_names"].is_null()) {
    channels_json = &extra_info["channel_names"];
  } else if (metadata.contains("channel_names") &&
             !metadata["channel_names"].is_null()) {
    channels_json = &metadata["channel_names"];
  }

  if (channels_json) {
    num_channels = channels_json->size();
  } else if (metadata.contains("num_channels") &&
             !metadata["num_channels"].is_null()) {
    num_channels = metadata["num_channels"];
  }

  // Populate Perfetto JSON
  for (uint32_t i = 0; i < num_channels; ++i) {
    std::string channel_name;
    if (channels_json) {
      channel_name = (*channels_json)[i];
    } else {
      channel_name = "Channel_" + std::to_string(i + 1);
      std::cout << channel_name << "\n";
    }

    perfetto.push_back({{"name", "thread_name"},
                        {"ph", "M"},
                        {"pid", pid},
                        {"tid", i + 1},
                        {"args", {{"name", channel_name}}}});
  }

  // Convert metadata "markerTypes" into a std::vector of keys for easy/fast
  // access
  if (extra_info.contains("markerTypes") &&
      !extra_info["markerTypes"].is_null()) {
    for (auto &[key, value] : extra_info["markerTypes"].items()) {
      markerTypes_values.push_back(value);
    }
  } else if (metadata.contains("markerTypes") &&
             !metadata["markerTypes"].is_null()) {
    for (auto &[key, value] : metadata["markerTypes"].items()) {
      markerTypes_values.push_back(value);
    }
  }

  return num_channels;
}

/**
 *
 */
int write_perfetto_json(const fs::path &base_path, nlohmann::json &perfetto) {
  std::ofstream out(base_path / "perfetto.json");

  if (!out.is_open()) {
    std::cerr << "Failed to open 'perfetto.json' for writing!\n";
    return 1;
  }

  // Dump JSON into file (pretty-printed with 4 spaces)
  out << perfetto.dump(4);
  out.close();

  std::cout << "perfetto.json written successfully.\n";
  return 0;
}

/**
 * Store in Perfetto format
 */
int perfetto(const std::vector<std::vector<TraCR::Payload>> &bts_files,
             const std::vector<pid_t> &bts_tids, nlohmann::json &extra_info,
             nlohmann::json &metadata, const fs::path base_path, int &pid) {

  // perfetto json array
  nlohmann::json perfetto = nlohmann::json::array();

  // if PID has not yet been set (i.e. -1), use 0
  if (pid == -1) {
    pid = 0;
  }

  // Define channel names in Perfetto
  std::vector<std::string> markerTypes_values;
  uint32_t num_channels = populate_perfetto_channels(
      extra_info, metadata, pid, perfetto, markerTypes_values);

  // Now we have to travers the map of all the std::vector<Payload>
  bool first = true;
  uint64_t start_time = uint64_t(metadata["start_time"]);
  std::vector<size_t> bts_files_ptrs(bts_files.size(), 0);
  std::vector<TraCR::Payload> prev_payload(
      num_channels, TraCR::Payload{0, UINT16_MAX, UINT32_MAX, 0});
  bool ptrs_end = false;
  while (!ptrs_end) {
    TraCR::Payload payload;
    size_t index = find_next_payload(bts_files, bts_files_ptrs, payload);

    if (first) {
      first = false;
      start_time = payload.timestamp;
    }

    if (prev_payload[payload.channelId].timestamp != 0) {
      uint16_t channelId = payload.channelId;
      std::string mType, colorId;

      colorId = prev_payload[channelId].eventId == UINT16_MAX
                    ? "rail_idle"
                    : perfetto_colors[prev_payload[channelId].eventId % 7];

      if (markerTypes_values.size() > 0) {
        mType = prev_payload[channelId].eventId == UINT16_MAX
                    ? ""
                    : markerTypes_values[prev_payload[channelId].eventId];
      } else {
        mType = std::to_string(prev_payload[channelId].eventId);
      }

      perfetto.push_back(
          {{"name", mType},
           {"cat", mType},
           {"ph", "X"},
           {"ts", (prev_payload[channelId].timestamp - start_time) / 1000.0},
           {"dur",
            (payload.timestamp - prev_payload[channelId].timestamp) / 1000.0},
           {"pid", pid},
           {"tid", prev_payload[channelId].channelId + 1},
           {"cname", colorId}});
    }
    prev_payload[payload.channelId] = payload;

    // check if all ptrs are at the ending
    ptrs_end = advance_ptrs_and_check_end(bts_files_ptrs, bts_files, index);
  }

  // Now we assert that all the last payloads of all are resets.
  // This is important, as otherwise we have missed the last set trace
  // information.
  for (size_t i = 0; i < bts_files.size(); ++i) {
    TraCR::Payload payload = bts_files[i][bts_files[i].size() - 1];
    if (payload.eventId != UINT16_MAX) {
      std::cout << "Warning: the last event got lost of this thread: "
                << bts_tids[i]
                << " has to be a INSTRUMENTATION_MARK_RESET() for perfetto "
                   "format!\n";
      return 1;
    }
  }

  // Create and open the metadata.json file
  if (write_perfetto_json(base_path, perfetto) != 0) {
    return 1;
  }

  return 0;
}

/**
 *  The main function to transform bts files into readable files
 *
 *  Use:
 *  1. Create a perfetto format file:
 *    - ./tracr_process <path-to-tracr/>
 *    - ./tracr_process <path-to-tracr/> perfetto
 *
 *  2. Create a perfetto format file:
 *    - ./tracr_process <path-to-tracr/> paraver
 *
 *  3. Pass extra informations about "channel_names" and/or "markerTypes"
 *    - ./tracr_process <path-to-tracr/> perfetto extra_info.json
 *    - ./tracr_process <path-to-tracr/> paraver extra_info.json
 */
int main(int argc, char *argv[]) {
  // Pass the tracr file and other additional arguments
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <folder_path>\n";
    return 1;
  }

  const fs::path base_path = argv[1];

  if (!fs::exists(base_path) || !fs::is_directory(base_path)) {
    std::cerr << "Error: Folder does not exist or is not a directory.\n";
    return 1;
  }

  // Optional: Choose "paraver" or "perfetto" format, default "perfetto"
  bool paraver_format = false;
  if (argc > 2) {
    std::string type = argv[2];

    if (type == "paraver") {
      paraver_format = true;
    } else if (type == "perfetto") {
      paraver_format = false;
    }
  }

  // Optional: Provide "channel_names" and/or "markerTypes" directly by the
  // user. if they are not provided by the metadata.json
  nlohmann::json extra_info;
  if (argc > 3) {
    if (get_extra_info(extra_info, argv[3]) != 0) {
      std::cerr << "get_extra_info() failed\n";
      return 1;
    };
  }

  // A container to keep all the bts files in one for the proc
  std::vector<std::vector<TraCR::Payload>> bts_files;
  std::vector<pid_t> bts_tids;

  // The metada of the proc
  nlohmann::json metadata;

  // TraCR Proc PID placeholder
  int pid = -1;

  /**
   * Iterate over all entries in the base folder
   *
   * NOTE: multiple proc.* are currenttly not yet allowed, the code will
   * terminate if this is the case.
   */
  if (extract_bts_metadata(bts_files, bts_tids, metadata, base_path, pid) !=
      0) {
    std::cerr << "extract_bts_metadata() failed\n";
    return 1;
  }

  if (paraver_format) {
    if (paraver(bts_files, bts_tids, extra_info, metadata, base_path, pid) !=
        0) {
      std::cerr << "paraver() failed\n";
      return 1;
    }
  } else {
    if (perfetto(bts_files, bts_tids, extra_info, metadata, base_path, pid) !=
        0) {
      std::cerr << "perfetto() failed\n";
      return 1;
    }
  }

  return 0;
}
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
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <tracr/marker_management_engine.hpp>

namespace fs = std::filesystem;

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
 * Type of tracr file processing format
 */
enum class Format { PARAVER, DUMP, PERFETTO };

/**
 * string to enum format for switch case
 */
Format parseFormat(const std::string &format) {
  if (format == "paraver")
    return Format::PARAVER;
  if (format == "dump")
    return Format::DUMP;
  return Format::PERFETTO; // default
}

/**
 * Properly quotes and escapes a string as a JSON string literal.
 */
static std::string json_str(const std::string &s) {
  return nlohmann::json(s).dump();
}

/**
 * Formats a nanosecond duration as a fixed-point microsecond string with
 * exactly 3 decimal places (e.g. 1017990020 ns -> "1017990.020").
 * Pure integer arithmetic avoids scientific notation and float precision loss.
 */
static std::string fmt_us(uint64_t ns) {
  char buf[32];
  std::snprintf(buf, sizeof(buf), "%llu.%03llu",
                (unsigned long long)(ns / 1000),
                (unsigned long long)(ns % 1000));
  return buf;
}

/**
 * Min-heap based k-way merge over a collection of pre-sorted Payload vectors.
 * Replaces the O(N*K) linear-scan approach with O(N*log K).
 */
class PayloadMerger {
  struct Entry {
    uint64_t timestamp;
    size_t file_idx;
    bool operator>(const Entry &o) const { return timestamp > o.timestamp; }
  };

  const std::vector<std::vector<TraCR::Payload>> &_files;
  std::vector<size_t> _ptrs;
  std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> _heap;

public:
  explicit PayloadMerger(const std::vector<std::vector<TraCR::Payload>> &files)
      : _files(files), _ptrs(files.size(), 0) {
    for (size_t i = 0; i < files.size(); ++i)
      if (!files[i].empty())
        _heap.push({files[i][0].timestamp, i});
  }

  bool empty() const { return _heap.empty(); }

  // Returns the next payload in timestamp order and the index of its source
  // file.
  std::pair<TraCR::Payload, size_t> next() {
    auto top = _heap.top();
    _heap.pop();
    TraCR::Payload p = _files[top.file_idx][_ptrs[top.file_idx]];
    if (++_ptrs[top.file_idx] < _files[top.file_idx].size())
      _heap.push(
          {_files[top.file_idx][_ptrs[top.file_idx]].timestamp, top.file_idx});
    return {p, top.file_idx};
  }
};

/**
 * A function to load a bts file into a std::vector<Payload>
 */
bool load_bts_file(const fs::path &filepath,
                   std::vector<TraCR::Payload> &traces) {
  std::ifstream ifs(filepath, std::ios::binary);
  if (!ifs) {
    std::cerr << "Failed to open file: " << filepath << "\n";
    return false;
  }

  ifs.seekg(0, std::ios::end);
  std::streamsize filesize = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  size_t count = filesize / sizeof(TraCR::Payload);
  traces.resize(count);

  ifs.read(reinterpret_cast<char *>(traces.data()),
           count * sizeof(TraCR::Payload));
  if (!ifs) {
    std::cerr << "Failed to read all data from file: " << filepath << "\n";
    return false;
  }

  return true;
}

/**
 * Goes through all the thread folder and loads all the bts files from
 * the given proc folder
 */
int load_thread_traces(const fs::path &proc_path,
                       std::vector<std::vector<TraCR::Payload>> &bts_files,
                       std::vector<pid_t> &bts_tids) {
  size_t tot_num_traces = 0;
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

    std::vector<TraCR::Payload> traces;

    if (!load_bts_file(trace_file, traces)) {
      std::cerr << "  Failed to load bts file: " << trace_file << "\n";
      return 1;
    }

    tot_num_traces += traces.size();

    std::cout << "Loaded " << traces.size() << " traces from " << trace_file
              << "\n";

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

  std::cout << "Total number of traces: " << tot_num_traces << "\n";

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
    std::cout << "  Loaded metadata.json:\n";
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

      if (load_metadata_json(proc_entry.path(), metadata) != 0) {
        return 1;
      }

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
                     const nlohmann::json &metadata) {
  std::ofstream out(base_path / "tracr.pcf");
  if (!out) {
    std::cerr << "Error opening tracr.pcf for writing\n";
    return 1;
  }

  out << PARAVER_HEADER;

  const nlohmann::json *markerTypes_json = nullptr;

  if (metadata.contains("markerTypes") && !metadata["markerTypes"].is_null()) {
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
void extract_channel_info(const nlohmann::json &metadata, size_t &num_channels,
                          std::stringstream &ss) {
  num_channels = 1; // default

  if (metadata.contains("channel_names") &&
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
std::vector<std::string> extract_marker_keys(const nlohmann::json &metadata) {
  std::vector<std::string> markerTypes_keys;

  const nlohmann::json *marker_json = nullptr;

  if (metadata.contains("markerTypes") && !metadata["markerTypes"].is_null()) {
    marker_json = &metadata["markerTypes"];
  }

  if (marker_json) {
    for (auto &[key, value] : marker_json->items()) {
      markerTypes_keys.push_back(key);
    }
  }

  return markerTypes_keys;
}

/**
 * Create the tracr.prv file for Paraver format
 */
int create_tracr_prv(const fs::path &base_path, const nlohmann::json &metadata,
                     const std::vector<std::vector<TraCR::Payload>> &bts_files,
                     size_t &num_channels, std::stringstream &ss) {
  std::ofstream out(base_path / "tracr.prv");
  if (!out) {
    std::cerr << "Error opening tracr.prv for writing\n";
    return 1;
  }

  extract_channel_info(metadata, num_channels, ss);

  auto now = std::chrono::system_clock::now();
  std::time_t now_time = std::chrono::system_clock::to_time_t(now);
  std::tm *local_tm = std::localtime(&now_time);

  out << "#Paraver (" << std::setw(2) << std::setfill('0') << local_tm->tm_mday
      << "/" << std::setw(2) << std::setfill('0') << (local_tm->tm_mon + 1)
      << "/" << std::setw(2) << std::setfill('0') << (local_tm->tm_year % 100)
      << " at " << std::setw(2) << std::setfill('0') << local_tm->tm_hour << ":"
      << std::setw(2) << std::setfill('0') << local_tm->tm_min
      << "):00000000000000000000_ns:0:1:1(" << num_channels << ":1)\n";

  std::vector<std::string> markerTypes_keys = extract_marker_keys(metadata);

  bool first = true;
  uint64_t start_time = 0;

  PayloadMerger merger(bts_files);
  while (!merger.empty()) {
    auto [payload, index] = merger.next();

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
            const std::vector<pid_t> &bts_tids, nlohmann::json &metadata,
            const fs::path base_path, int &pid) {
  if (copy_state_cfg(base_path) != 0) {
    return 1;
  }

  if (create_tracr_pcf(base_path, metadata) != 0) {
    return 1;
  }

  size_t num_channels = 1;
  std::stringstream ss;
  if (create_tracr_prv(base_path, metadata, bts_files, num_channels, ss) != 0) {
    return 1;
  }

  if (create_tracr_row(base_path, num_channels, ss) != 0) {
    return 1;
  }

  return 0;
}

/**
 *
 */
void validate_last_events_for_perfetto(
    const std::vector<std::vector<TraCR::Payload>> &bts_files,
    const std::vector<pid_t> &bts_tids) {
  for (size_t i = 0; i < bts_files.size(); ++i) {

    if (bts_files[i].empty())
      continue;

    const TraCR::Payload &payload = bts_files[i].back();

    if (payload.eventId != UINT16_MAX) {
      std::cout
          << "WARNING: the last event got lost of this thread: " << bts_tids[i]
          << ". To not loose this last event add INSTRUMENTATION_MARK_RESET() "
             "as the last event for Perfetto format.\n";
    }
  }
}

/**
 * Store in Perfetto format.
 *
 * Writes a JSON object with a "traceEvents" array in nanoseconds. Events are
 * streamed directly to disk — no in-memory JSON array is built.
 */
int perfetto(const std::vector<std::vector<TraCR::Payload>> &bts_files,
             const std::vector<pid_t> &bts_tids, nlohmann::json &metadata,
             const fs::path base_path, int &pid) {

  if (pid == -1)
    pid = 0;

  // Extract channel count and names
  uint32_t num_channels = 1;
  const nlohmann::json *channels_json = nullptr;
  if (metadata.contains("channel_names") &&
      !metadata["channel_names"].is_null())
    channels_json = &metadata["channel_names"];
  if (channels_json)
    num_channels = channels_json->size();
  else if (metadata.contains("num_channels") &&
           !metadata["num_channels"].is_null())
    num_channels = metadata["num_channels"];

  // Extract marker type labels for event name lookup
  std::vector<std::string> markerTypes_values;
  if (metadata.contains("markerTypes") && !metadata["markerTypes"].is_null())
    for (auto &[key, value] : metadata["markerTypes"].items())
      markerTypes_values.push_back(value);

  std::ofstream out(base_path / "perfetto.json");
  if (!out.is_open()) {
    std::cerr << "Failed to open 'perfetto.json' for writing!\n";
    return 1;
  }

  // Wrapper object: traceEvents array + ns time unit
  out << "{\"traceEvents\":[\n";

  // Process name metadata event (first entry — no leading comma)
  out << "{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":" << pid
      << ",\"args\":{\"name\":\"TraCR\"}}";

  // Thread name metadata events (one per channel)
  for (uint32_t i = 0; i < num_channels; ++i) {
    std::string name = channels_json ? std::string((*channels_json)[i])
                                     : ("Channel_" + std::to_string(i + 1));
    out << ",\n{\"name\":\"thread_name\",\"ph\":\"M\",\"pid\":" << pid
        << ",\"tid\":" << (i + 1) << ",\"args\":{\"name\":" << json_str(name)
        << "}}";
  }

  // Merge all thread buffers in timestamp order and emit complete events (ph=X)
  bool first = true;
  uint64_t start_time = 0;
  std::vector<TraCR::Payload> prev_payloads(
      num_channels, TraCR::Payload{0, UINT16_MAX, UINT32_MAX, 0});

  PayloadMerger merger(bts_files);
  while (!merger.empty()) {
    auto [payload, index] = merger.next();

    if (first) {
      first = false;
      start_time = payload.timestamp;
    }

    uint16_t channelId = payload.channelId;
    if (channelId >= num_channels) {
      std::cerr << "Payload channelId " << channelId << " is out of bounds!\n";
      return 1;
    }

    if (prev_payloads[channelId].eventId != UINT16_MAX) {
      const TraCR::Payload &prev = prev_payloads[channelId];
      std::string mType = (!markerTypes_values.empty())
                              ? markerTypes_values[prev.eventId]
                              : std::to_string(prev.eventId);

      out << ",\n{\"name\":" << json_str(mType)
          << ",\"cat\":\"tracr\",\"ph\":\"X\""
          << ",\"ts\":" << fmt_us(prev.timestamp - start_time)
          << ",\"dur\":" << fmt_us(payload.timestamp - prev.timestamp)
          << ",\"pid\":" << pid << ",\"tid\":" << (prev.channelId + 1);
      if (prev.extraId != UINT32_MAX)
        out << ",\"args\":{\"extra_id\":" << prev.extraId << "}";
      out << "}";
    }

    prev_payloads[channelId] = payload;
  }

  validate_last_events_for_perfetto(bts_files, bts_tids);

  out << "\n]}\n";
  out.close();

  std::cout << "perfetto.json written successfully.\n";
  return 0;
}

/**
 * Dump trace info to terminal
 */
int dump_info(const std::vector<std::vector<TraCR::Payload>> &bts_files,
              const std::vector<pid_t> &bts_tids, const fs::path base_path) {

  std::unordered_map<uint16_t, int32_t> channelIds_check;
  std::unordered_map<uint16_t, std::unordered_set<uint32_t>> extraIds_check;

  std::cout << "Thread[x]: [channelId, eventId, extraId, timestamp]\n";

  PayloadMerger merger(bts_files);
  while (!merger.empty()) {
    auto [payload, index] = merger.next();

    std::cout << "Thread[" << bts_tids[index] << "]: [" << payload.channelId
              << ", " << payload.eventId << ", " << payload.extraId << ", "
              << payload.timestamp << "]\n";

    int32_t &counter = channelIds_check[payload.channelId];
    if (payload.eventId == UINT16_MAX) {
      --counter;
    } else {
      ++counter;
    }

    auto &inner_set = extraIds_check[payload.channelId];
    auto it_inner = inner_set.find(payload.extraId);
    if (it_inner == inner_set.end()) {
      inner_set.insert(payload.extraId);
    } else {
      inner_set.erase(it_inner);
    }
  }

  std::cout << "\nChannels which do not follow Push/Pop methology: {channelId, "
               "count}\n";
  for (const auto &[key, value] : channelIds_check) {
    if (value != 0) {
      std::cout << "{" << key << ", " << value << "}\n";
    }
  }

  std::cout << "\nChannels which do not follow Push/Pop methology for the "
               "extraIds: channelId: {extraIds...}\n";
  for (const auto &[channelId, inner_set] : extraIds_check) {
    if (inner_set.size() > 0) {
      std::cout << channelId << ": {";
      for (const auto &value : inner_set) {
        std::cout << value << ", ";
      }
      std::cout << "}\n";
    }
  }
  std::cout << "\n";

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
 *  2. Create a paraver format file:
 *    - ./tracr_process <path-to-tracr/> paraver
 *
 *  3. Dump traces and informations directly in the terminal (for debugging):
 *    - ./tracr_process <path-to-tracr/> dump
 */
int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 3) {
    std::cerr << "Usage: " << argv[0] << " <folder_path>\n OR " << argv[0]
              << " <folder_path>" << "<'perfetto'|'paraver'|'dump'>\n";
    return 1;
  }

  const fs::path base_path = argv[1];

  if (!fs::exists(base_path) || !fs::is_directory(base_path)) {
    std::cerr << "Error: Folder does not exist or is not a directory.\n";
    return 1;
  }

  std::string format = "perfetto";
  if (argc == 3) {
    format = argv[2];
  }

  std::vector<std::vector<TraCR::Payload>> bts_files;
  std::vector<pid_t> bts_tids;
  nlohmann::json metadata;
  int pid = -1;

  if (extract_bts_metadata(bts_files, bts_tids, metadata, base_path, pid) !=
      0) {
    std::cerr << "extract_bts_metadata() failed\n";
    return 1;
  }

  switch (parseFormat(format)) {
  case Format::PARAVER:
    if (paraver(bts_files, bts_tids, metadata, base_path, pid) != 0) {
      std::cerr << "paraver() failed\n";
      return 1;
    }
    break;
  case Format::DUMP:
    if (dump_info(bts_files, bts_tids, base_path) != 0) {
      std::cerr << "dump_info() failed\n";
      return 1;
    }
    break;
  case Format::PERFETTO:
    if (perfetto(bts_files, bts_tids, metadata, base_path, pid) != 0) {
      std::cerr << "perfetto() failed\n";
      return 1;
    }
    break;
  }

  std::cout << "TraCR Process finished successfully\n";

  return 0;
}

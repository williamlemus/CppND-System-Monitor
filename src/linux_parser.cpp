#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;


int readProcessStat(string token) {
  string line;
  std::ifstream stream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
  if(stream.is_open()){
    while(std::getline(stream, line)) {
      if(line.find(token) != std::string::npos) {
        return stoi(line.substr(string(token).size()));
      }
    }
  }
  return 0;
}




// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  // Added version to parse fedora correctly
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line;
  float memTotal = 0;
  float memFree = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if(stream.is_open()){
    while(std::getline(stream, line)) {
      if(line.find("MemTotal:") != std::string::npos) {
        memTotal = stof(line.substr(string("MemTotal:").size()));
      } else if(line.find("MemFree:") != std::string::npos) {
        memFree = stof(line.substr(string("MemFree:").size()));
      }
   }
  }

  return (memTotal - memFree)/ memTotal;
 }

long LinuxParser::UpTime() { 
  string timeInSeconds;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if(stream.is_open()){
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> timeInSeconds;
  }
  return stoi(timeInSeconds);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid[[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

// TODO: do a wait per here: https://stackoverflow.com/a/23376195
// You'll want to do a helper function that parses the file
vector<string> LinuxParser::CpuUtilization() {
  vector<string> result{};
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if(stream.is_open()) {
    // Can expand this to get various processors
    string temp;
    float utilization;
    int idle, nonIdle, total;
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> temp;
    vector<int> fields;
    while(linestream >> temp){
      fields.push_back(stoi(temp));
    }
    // idle = idle + iowait
    idle = fields[3] + fields[4];
    // nonIdle = user + nice + system + irq + softirq + steal
    nonIdle = fields[0] + fields[1] + fields[2] + fields[5] + fields[6] + fields[7];
    total = idle + nonIdle;
    utilization = (total - idle)/(float)total;
    result.push_back(to_string(utilization));
  }
  return result;
}

float LinuxParser::CpuUtilization(int pid) {
  string line;
  vector<string> columns;
  string column;
  float utilization{0.0};
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    getline(stream, line);
    std::istringstream linestream(line);
    while(linestream.good()) {
      getline(linestream, column, ' ');
      columns.push_back(column);
    }
    //totalTime = utime + stime
    // with child processes totalTime += cutime + cstime
    int totalTime = stoi(columns[13]) + stoi(columns[14]) + stoi(columns[15]) + stoi(columns[16]);
    long totalSeconds = UpTime() - UpTime(pid);
    utilization = totalSeconds != 0 ? 100.0 * ((totalTime/sysconf(_SC_CLK_TCK))/totalSeconds) : 0.0;
  }
  return utilization;
}

int LinuxParser::TotalProcesses() { 
  return readProcessStat("processes");
}

int LinuxParser::RunningProcesses() {   
  return readProcessStat("procs_running");
}

string LinuxParser::Command(int pid) {
  string result;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if(stream.is_open()) {
    std::getline(stream, result);
  }
  return result;
}

string LinuxParser::Ram(int pid) {
  string line;
  int kilobytes{0};

  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      if(line.find("VmSize:") != std::string::npos) {
        kilobytes = stoi(line.substr(string("VmSize:").size()));
        return to_string(kilobytes/1000);
      }
    }
  }
  return to_string(kilobytes/1000);
}

string LinuxParser::Uid(int pid) {
  string line;
  string uid;
  string key;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      if(line.find("Uid:") != std::string::npos) {
        std::istringstream linestream(line);
        linestream >> key >> uid;
        return uid;
      }
    }
  }
  return uid;
 }

string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line;
  std::ifstream stream(kPasswordPath);
  if(stream.is_open()) {
    while(std::getline(stream, line)) {
      std::stringstream linestream(line);
      string username;
      if(linestream.good()){
        getline(linestream, username, ':');
      }
      for(int i = 0; i < 2; ++i) {
        if(linestream.good()) {
           string columnValue;
           getline(linestream, columnValue, ':');
          if(i == 1 && columnValue == uid) {
              return username;
          }
        }
      }
    }
  }

  return "";
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  string line;
  string column;
  int uptimeInTicks;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if(stream.is_open()) {
    getline(stream, line);
    std::istringstream linestream(line);
    for(int i = 0; i < 22; ++i) {
      linestream >> column;
    }
  }
  uptimeInTicks = stoi(column);
  return uptimeInTicks/sysconf(_SC_CLK_TCK);
}
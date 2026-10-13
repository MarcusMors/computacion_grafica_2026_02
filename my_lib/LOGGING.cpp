#include "LOGGING.hpp"

#if ENABLE_LOGGING


#include <chrono>
#include <csignal>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
ofstream log_file;

void HandleInterruptSignal(int signal)
{
  if (log_file.is_open()) { log_file.close(); }
  std::exit(signal);
}


std::string log_path()
{
  auto now = chrono::system_clock::now();
  const time_t currentTime = chrono::system_clock::to_time_t(now);

  const tm *local_time = localtime(&currentTime);// Convert the time_t object to a struct tm object
  ostringstream file_name;
  file_name << put_time(local_time, "log__%Y-%m-%d__%H-%M-%S.log");
  string logs_dir = "../logs/";
  string log_path = logs_dir + file_name.str();
  return log_path;
}
#endif

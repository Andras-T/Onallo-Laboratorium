#include "Logger.h"
using Clock = std::chrono::system_clock;

const std::ostringstream time() {
    auto currentTime = Clock::now();
    auto time = Clock::to_time_t(currentTime);

    std::tm tm = *std::localtime(&time);

    // Extract year, month, day, hour, minute, and second
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon + 1; // Months are 0-based
    int day = tm.tm_mday;
    int hour = tm.tm_hour;
    int minute = tm.tm_min;
    int second = tm.tm_sec;

    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        currentTime.time_since_epoch())
        .count() %
        1000;

    // Format the time string
    std::ostringstream formattedTime;

    formattedTime << std::setfill('0');
    formattedTime << std::setw(4) << year << ":";
    formattedTime << std::setw(2) << month << ":";
    formattedTime << std::setw(2) << day << " ";
    formattedTime << std::setw(2) << hour << ":";
    formattedTime << std::setw(2) << minute << " ";
    formattedTime << std::setw(2) << second << "s ";
    formattedTime << std::setw(3) << milliseconds << "ms";

    return formattedTime;
}

void Logger::LogInfo(const std::string &log, const std::string &prefix,
                     const std::string_view &color) {
  auto t = time().str();
  auto indentation = t.size() + prefix.size() + 2;
  auto logStr = formatMessage(log, false, indentation).str();
  std::cout << "\x1b[36m" << t << endFormat << "\x1b[32m " << prefix
            << endFormat << " " << color << logStr << endFormat << "\n";
}

void Logger::LogDiag(const std::string &log, const std::string &prefix,
                     const std::string_view &color) {
  if (severity > Info) {
    auto t = time().str();
    auto indentation = t.size() + prefix.size() + 2;
    auto logStr = formatMessage(log, false, indentation).str();
    std::cout << "\x1b[36m" << t << endFormat << "\x1b[32m " << prefix
              << endFormat << " " << color << logStr << endFormat << "\n";
  }
}

void Logger::LogTrace(const std::string &log, const std::string &prefix,
                      const std::string_view &color) {
  if (severity == Trace) {
    auto t = time().str();
    auto indentation = t.size() + prefix.size() + 2;
    auto logStr = formatMessage(log, false, indentation).str();
    std::cout << "\x1b[36m" << t << endFormat << "\x1b[32m " << prefix
              << endFormat << " " << color << logStr << endFormat << "\n";
  }
}

void Logger::LogError(const std::string &log, const std::string &prefix) {
  auto t = time().str();
  auto indentation = t.size() + prefix.size() + 2;
  auto logStr = formatMessage(log, false, indentation).str();
  std::cout << "\x1b[36m" << t << endFormat << "\x1b[32m " << prefix
            << endFormat << " "
            << "\x1b[31m" << logStr << endFormat << "\n";
}

void Logger::LogWarning(const std::string& log, const std::string& prefix)
{
    auto t = time().str();
    auto indentation = t.size() + prefix.size() + 2;
    auto logStr = formatMessage(log, false, indentation).str();
    std::cout << "\x1b[36m" << t << endFormat << "\x1b[32m " << prefix
        << endFormat << " "
        << "\x1B[33m" << logStr << endFormat << "\n";
}

std::ostringstream Logger::formatMessage(const std::string &messageStr,
                                         bool addDoubleSpace, int indentation,
                                         int lineLimit) {
  lineLimit -= indentation;
  size_t startPos = 0, length = messageStr.length();

  std::ostringstream formattedMsg{};
  while (startPos < length) {
    std::string line = messageStr.substr(startPos, lineLimit);
    if (startPos + lineLimit < length) {
      // Find the last space in the line to break it at word boundaries
      size_t lastSpace = line.find_last_of(" ");
      if (lastSpace != std::string::npos) {
        line = line.substr(0, lastSpace);
        startPos += lastSpace + 1; // Skip the space
      } else {
        startPos += line.length();
      }
    } else {
      startPos += line.length();
    }
    formattedMsg << line;
    if ((startPos) < length)
      formattedMsg << "\n" << std::string(indentation, ' ');
  }

  if (addDoubleSpace) {
    formattedMsg << "\n";
  }
  return formattedMsg;
}
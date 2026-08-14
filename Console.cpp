#include "Console.h"

#include <cstdlib>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {

std::string trim(const std::string& text) {
  const std::string whitespace = " \t\r\n";
  const std::string::size_type first = text.find_first_not_of(whitespace);
  if (first == std::string::npos) return std::string();
  const std::string::size_type last = text.find_last_not_of(whitespace);
  return text.substr(first, last - first + 1);
}

}  // namespace

namespace Console {

void clear() {
#ifdef _WIN32
  std::system("cls");
#else
  std::system("clear");
#endif
}

void sleepMs(unsigned int milliseconds) {
#ifdef _WIN32
  Sleep(milliseconds);
#else
  usleep(milliseconds * 1000);
#endif
}

void pause(const std::string& message) {
  std::cout << std::endl << message;
  std::string ignored;
  std::getline(std::cin, ignored);
}

bool readLine(const std::string& prompt, std::string& out) {
  for (;;) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
      std::cin.clear();
      return false;
    }
    line = trim(line);
    if (!line.empty()) {
      out = line;
      return true;
    }
    std::cout << "  Value cannot be empty." << std::endl;
  }
}

bool readInt(const std::string& prompt, int min, int max, int& out) {
  for (;;) {
    std::cout << prompt;
    std::string line;
    if (!std::getline(std::cin, line)) {
      std::cin.clear();
      return false;
    }

    std::istringstream stream(trim(line));
    int value = 0;
    char trailing = '\0';
    if ((stream >> value) && !(stream >> trailing)) {
      if (value >= min && value <= max) {
        out = value;
        return true;
      }
    }
    std::cout << "  Please enter a whole number between " << min << " and "
              << max << "." << std::endl;
  }
}

void heading(const std::string& title) {
  std::cout << std::endl << title << std::endl;
  std::cout << std::string(title.size(), '*') << std::endl << std::endl;
}

}  // namespace Console

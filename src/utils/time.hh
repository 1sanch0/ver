#ifndef TIME_H_
#define TIME_H_

#include <string>
#include <chrono>

namespace utils {
  namespace time {
    std::string format(std::chrono::milliseconds millis);

  }
}

#endif // TIME_H_

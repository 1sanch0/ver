#include "time.hh"
#include <iomanip>
#include <sstream>

namespace utils {
  namespace time {
    std::string format(std::chrono::milliseconds millis) {
      auto hours = std::chrono::duration_cast<std::chrono::hours>(millis);
      millis -= hours;
      auto minutes = std::chrono::duration_cast<std::chrono::minutes>(millis);
      millis -= minutes;
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(millis);

      std::stringstream ss;

      ss << std::setfill('0');
      ss << std::setw(2) << hours.count() << ":";
      ss << std::setw(2) << minutes.count() << ":";
      ss << std::setw(2) << seconds.count() << ".";
      ss << std::setw(2) << millis.count();

      return ss.str();
    }

  }
}
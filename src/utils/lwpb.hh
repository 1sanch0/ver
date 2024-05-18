#ifndef LWPB_H_
#define LWPB_H_

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <chrono>
#include <cmath>

namespace utils {
  // Lightweight Progress Bar
  // very simple could be improved
  class lwpb {
    private:
      static unsigned int digits(unsigned int n) {
        return n > 0 ? static_cast<unsigned int>(std::log10(n)) + 1 : 1;
      }

    public:
      lwpb(unsigned int total_, unsigned int bar_length_ = 30)
        : total(total_), current(0), iter(0), bar_length(bar_length_),
          start(std::chrono::high_resolution_clock::now()) {}

      void step(unsigned int n = 1) {
        current += n;
        iter++;
      }

      void print(std::ostream& os = std::cout) const {
        std::stringstream ss;

        const unsigned int percent = (current * 100) / total;
        const std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
        const double eta = elapsed.count() / current * (total - current);

        ss << std::setw(3) << percent << "%|";

        for (unsigned int i = 0; i < bar_length; ++i)
          ss << (i < (current * bar_length) / total ? fill : empty);

        ss << "| " << std::setw(digits(total)) << current << "/" << total;
        ss << " [" << elapsed.count() << "s/" << eta << "s, " << static_cast<double>(iter) / elapsed.count() << "it/s]";

        const auto output = ss.str();
        os << output << std::flush;
        os << "\r" << std::string(output.size(), ' ') << "\r";
      }

      void update(unsigned int n = 1, std::ostream& os = std::cout) {
        step(n);
        print(os);
      }

    private:
      unsigned int total;
      unsigned int current, iter;
      unsigned int bar_length;

      std::chrono::time_point<std::chrono::high_resolution_clock> start;

      static const char fill  = '#';
      static const char empty = ' ';
  };
} // namespace utils


#endif // LWPB_H_

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
      lwpb(unsigned int total_, const std::string &description = "", unsigned int bar_length_ = 30)
        : total(total_), current(0), iter(0), bar_length(bar_length_),
          desc(description), start(std::chrono::high_resolution_clock::now()) {
        if (desc.size() > 0) desc += ": ";
      }
      
      void setDescription(const std::string &description) {
        desc = description;
      }

      void step(unsigned int n = 1) {
        current += n;
        iter++;
      }

      void print(std::ostream& os = std::cout) const {
        std::stringstream ss;

        const unsigned int percent = (current * 100) / total;
        const std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
        const double eta = elapsed.count() / current * (total - current);

        ss << "\r\033[K";
        ss << desc << std::setw(3) << percent << "% |";

        for (unsigned int i = 0; i < bar_length; ++i)
          ss << (i < (current * bar_length) / total ? fill : empty);

        ss << "| " << std::setw(digits(total)) << current << "/" << total;
        ss << std::fixed << std::setprecision(2);
        ss << " [" << elapsed.count() << "s/" << eta << "s, " << static_cast<double>(iter) / elapsed.count() << "it/s]";

        os << ss.str() << std::flush;
        if (current == total) os << std::endl;
      }

      void update(unsigned int n = 1, std::ostream& os = std::cout) {
        step(n);
        print(os);
      }

    private:
      unsigned int total;
      unsigned int current, iter;
      unsigned int bar_length;

      std::string desc;

      std::chrono::time_point<std::chrono::high_resolution_clock> start;

      static const char fill  = '#';
      static const char empty = ' ';
  };
} // namespace utils


#endif // LWPB_H_

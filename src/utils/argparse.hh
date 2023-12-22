#ifndef ARGPARSE_H_
#define ARGPARSE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
  typedef unsigned int uint;
#endif

namespace utils {
  class ArgumentParser {
    public:
      class Argument {
        public:
          Argument(const std::string &name, const std::string &help = "");

          Argument &choices(const std::vector<std::string> &choices);
          Argument &default_value(const std::string &value);
          Argument &default_value(const std::vector<std::string> &value);
          Argument &flag();
          Argument &nargs(uint nargs);
          Argument &nargs(char nargs);
          Argument &help(const std::string &help);

        private:
          std::string hintChoices() const;
          std::string hintDefault() const;
          std::string hintNargs() const;
          std::string hintFlag() const;
          std::string hintHelp() const;

        private:
          struct nargs_t {
            uint min;
            uint max;
          };

        private:
          std::string name_;
          std::vector<std::string> choices_;
          std::vector<std::string> default_;
          nargs_t nargs_;
          std::string help_;

          bool isFlag;

          friend class ArgumentParser;
      };

    public:
      ArgumentParser(const std::string &prog, const std::string &description = "");

      Argument &addArgument(const std::string &name, const std::string &help = "");

      std::unordered_map<std::string, std::vector<std::string>> parse(int argc, char **argv) const;

      std::string usage() const;

    private:
      std::string program;
      std::string desc;

      std::vector<Argument> positional_arguments;
      std::vector<Argument> optional_arguments;
      std::set<std::string> names;
  };
}

#endif // ARGPARSE_H_

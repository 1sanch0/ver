#include "argparse.hh"
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace utils {
  ArgumentParser::Argument::Argument(const std::string &name, const std::string &help)
    : name_{name}, choices_{}, default_{}, nargs_{1, 1}, help_{help}, isFlag{false} {}

  ArgumentParser::Argument &ArgumentParser::Argument::choices(const std::vector<std::string> &choices) {
    this->choices_ = choices;
    return *this;
  }

  ArgumentParser::Argument &ArgumentParser::Argument::default_value(const std::string &value) {
    return default_value(std::vector<std::string>{value});
  }

  ArgumentParser::Argument &ArgumentParser::Argument::default_value(const std::vector<std::string> &value) {
    this->default_ = value;
    return *this;
  }

  ArgumentParser::Argument &ArgumentParser::Argument::flag() {
    this->isFlag = true;
    return *this;
  }

  ArgumentParser::Argument &ArgumentParser::Argument::nargs(uint nargs) {
    this->nargs_.min = nargs;
    this->nargs_.max = nargs;
    return *this;
  }

  ArgumentParser::Argument &ArgumentParser::Argument::nargs(char nargs) {
    switch (nargs) {
      case '?':
        this->nargs_.min = 0;
        this->nargs_.max = 1;
        break;
      case '*':
        this->nargs_.min = 0;
        this->nargs_.max = (uint)-1;
        break;
      case '+':
        this->nargs_.min = 1;
        this->nargs_.max = (uint)-1;
        break;
      default:
        throw std::runtime_error("Invalid nargs");
    }
    return *this;
  }

  ArgumentParser::Argument &ArgumentParser::Argument::help(const std::string &help) {
    this->help_ = help;
    return *this;
  }

  std::string ArgumentParser::Argument::hintChoices() const {
    std::string hint = "";
    if (choices_.size() > 0) {
      hint += " {";
      for (const auto &choice : choices_) {
        hint += choice + ",";
      }
      hint.pop_back();
      hint += "}";
    }
    return hint;
  }

  std::string ArgumentParser::Argument::hintDefault() const {
    std::string hint = "";
    if (default_.size() > 0) {
      hint += " (default: ";
      for (const auto &value : default_) {
        hint += value + ",";
      }
      hint.pop_back();
      hint += ")";
    }
    return hint;
  }

  std::string ArgumentParser::Argument::hintNargs() const {
    std::string hint = "";
    if (nargs_.min == 0 && nargs_.max == 1) {
      hint += " (optional)";
    } else if (nargs_.min == 0 && nargs_.max == (uint)-1) {
      hint += " (optional, multiple)";
    } else if (nargs_.min == 1 && nargs_.max == (uint)-1) {
      hint += " (multiple)";
    }
    return hint;
  }

  std::string ArgumentParser::Argument::hintFlag() const {
    std::string hint = "";
    if (isFlag)
      hint += " (flag)";
    return hint;
  }

  std::string ArgumentParser::Argument::hintHelp() const {
    std::string hint = "";
    if (!help_.empty()) {
      hint += " " + help_;
    } else {
      hint += " No help available";
    }
    return hint;
  }

  ArgumentParser::ArgumentParser(const std::string &prog, const std::string &description)
    : program{prog}, desc{description} {
      addArgument("-h", "Show this help message and exit").flag();
    }

  ArgumentParser::Argument &ArgumentParser::addArgument(const std::string &name, const std::string &help) {
    Argument arg(name, help);

    if (names.find(name) != names.end()) {
      throw std::runtime_error("Argument " + name + " already exists");
    }
    names.insert(name);

    if (name[0] == '-') {
      optional_arguments.push_back(arg);
      return optional_arguments.back();
    } else {
      positional_arguments.push_back(arg);
      return positional_arguments.back();
    }
  }

  std::unordered_map<std::string, std::vector<std::string>> ArgumentParser::parse(int argc, char **argv) const {
    std::unordered_map<std::string, std::vector<std::string>> args;

    for (const auto &arg : optional_arguments)
      args[arg.name_] = arg.default_;
    
    for (const auto &arg : positional_arguments)
      args[arg.name_] = arg.default_;
    
    uint positional = 0;
    for (int i = 1; i < argc; i++) {
      std::string arg = argv[i];

      if (arg == "-h") {
        std::cout << usage() << std::endl;
        exit(0);
      }

      if (arg[0] == '-') { // Optional 
        if (names.find(arg) == names.end())
          throw std::runtime_error("Unknown argument " + arg);
        
        const std::string name = arg;
        // Look for argument name in optional_arguments
        auto it = std::find_if(optional_arguments.begin(), optional_arguments.end(), 
          [&name](const Argument &a) { return a.name_ == name; });

        if (it == optional_arguments.end())
          throw std::runtime_error("Argument " + name + " not found");
        
        const bool isFlag = it->isFlag;
        const auto &choices = it->choices_;
        const uint max = it->nargs_.max;
        const uint min = it->nargs_.min;
        auto &values = args[name];
        values.clear(); // Clear default values

        if (isFlag) {
          values.push_back("true"); // TODO: Has to be the opposite of the default value
        } else {
          i++;
          for (uint j = 0; j < max && i < argc && argv[i][0] != '-'; i++, j++) {
            if (choices.size() > 0 && std::find(choices.begin(), choices.end(), argv[i]) == choices.end())
              throw std::runtime_error("Invalid value for argument " + name + ": " + argv[i] + " (expected one of" + it->hintChoices() + ")");
            values.push_back(argv[i]);
          }
          i--;
        }

        if (values.size() < min)
          throw std::runtime_error("Too few values for argument " + name);
        
        if (values.size() > max)
          throw std::runtime_error("Too many values for argument " + name);
      } else { // Positional
        if (positional >= positional_arguments.size())
          throw std::runtime_error("Too many positional arguments");

        const std::string name = positional_arguments[positional].name_; 
        const auto &choices = positional_arguments[positional].choices_;
        const uint max = positional_arguments[positional].nargs_.max;
        const uint min = positional_arguments[positional].nargs_.min;
        auto &values = args[name];
        values.clear(); // Clear default values

        for (uint j = 0; j < max && i < argc && argv[i][0] != '-'; i++, j++) {
          if (choices.size() > 0 && std::find(choices.begin(), choices.end(), argv[i]) == choices.end())
            throw std::runtime_error("Invalid value for argument " + name + ": " + argv[i] + " (expected one of" + positional_arguments[positional].hintChoices() + ")");
          values.push_back(argv[i]);
        }
        i--;

        if (values.size() < min)
          throw std::runtime_error("Too few values for argument " + name);
        
        if (values.size() > max)
          throw std::runtime_error("Too many values for argument " + name);
        
        positional++;
      }
    }

    for (const auto &arg : args) {
      const std::string name = arg.first;
      if (name == "-h") continue;

      auto it = std::find_if(optional_arguments.begin(), optional_arguments.end(), 
        [&name](const Argument &a) { return a.name_ == name; });

      uint min;
      if (it == optional_arguments.end()) {
        auto pit = std::find_if(positional_arguments.begin(), positional_arguments.end(), 
          [&name](const Argument &a) { return a.name_ == name; });
        
        if (pit == positional_arguments.end())
          throw std::runtime_error("This should not happen");
        
        min = pit->nargs_.min;
      } else {
        min = it->nargs_.min;
      }

      if (arg.second.size() < min)
        throw std::runtime_error("Missing value for argument " + arg.first);
    }

    return args;
  }

  std::string ArgumentParser::usage() const {
    std::stringstream usage;

    usage << "Usage: " << program;

    for (const auto &arg : optional_arguments) {
      usage << " [" << arg.name_;
      usage << arg.hintChoices();
      // usage << arg.hintDefault();
      usage << arg.hintNargs();
      usage << "]";
    }

    for (const auto &arg : positional_arguments) {
      std::stringstream optional;
      optional << arg.hintChoices();
      // optional << arg.hintDefault();
      optional << arg.hintNargs();

      if (optional.str().size() > 0) {
        usage << " [" << arg.name_ << optional.str() << "]";
      } else {
        usage << " " << arg.name_;
      }
    }

    usage << std::endl << std::endl;

    if (!desc.empty())
      usage << desc << std::endl << std::endl;

    usage << "Positional arguments:" << std::endl;
    for (const auto &arg : positional_arguments) {
      usage << " " << arg.name_;
      usage << std::setw(30 - arg.name_.size()) << " ";
      usage << arg.hintHelp();
      usage << arg.hintChoices();
      usage << arg.hintDefault();
      usage << arg.hintNargs();
      usage << arg.hintFlag();
      usage << std::endl;
    }

    usage << std::endl << "Optional arguments:" << std::endl;

    for (const auto &arg : optional_arguments) {
      usage << " " << arg.name_;
      usage << std::setw(30 - arg.name_.size()) << " ";
      usage << arg.hintHelp(); 
      usage << arg.hintChoices();
      usage << arg.hintDefault();
      usage << arg.hintNargs();
      usage << arg.hintFlag();
      usage << std::endl;
    }

    return usage.str();
  }
}
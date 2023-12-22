#include "ver.hh"
#include "io.hh"

#include <fstream>
#include <sstream>

// Removes all non-(alphanumeric+'.') characters
static std::string cleanString(const std::string &str) {
  std::string out;
  for (auto &c : str)
    if (isalnum(c) || c == '.')
      out += c;
  return out;
}

namespace image {
  Film read(const std::string &filename) {
    size_t width, height, color_resolution;
    double max;

    std::string magic, word, comment;
    std::stringstream header;

    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open())
      throw std::runtime_error("Failed to open file: " + filename + "\n");

    file >> magic;

    if (magic != "P3")
      throw std::runtime_error("Bad magic number encontered, expected P3 got: " + magic + "\n");

    // https://paulbourke.net/dataformats/ppm/#:~:text=A%20PPM%20file%20consists%20of,including%20the%20double%20quotes!).
    // To know more about the PPM format

    int nParams = 3; // width, height, color_resolution
    bool isMax = false;
    while (nParams > 0) {
      file >> word;
      if (word[0] == '#') {
        comment += word + "";
        std::getline(file, word);
        comment += word + "";

        comment = cleanString(comment);
        if (comment.substr(0,3) == "MAX") {
          max = stof(comment.substr(3, -1));
          // TODO: maybe alert if more than one found
          isMax = true;
        }
        comment = "";
      } else {
        header << word << "\n";
        nParams--;
      }
    }

    if (!isMax)
      throw std::runtime_error("Comment with maximum value of image not found!");

    header >> width >> height >> color_resolution;

    DEBUG_CODE({
      std::cout << "[IMAGE READ " << filename << "]:";
      std::cout << " width: " << width << ", height: " << height;
      std::cout << ", color_resolution: " << color_resolution;
      std::cout << ", max: " << max;
      std::cout << std::endl;
    });

    // Image data
    Film film(width, height, color_resolution);
    for (size_t i = 0; i < width * height; i++) {
      Pixel px;
      file >> px.r >> px.g >> px.b;

      px.r = px.r * max / color_resolution;
      px.g = px.g * max / color_resolution;
      px.b = px.b * max / color_resolution;

      film[i] = px;
    }

    file.close();

    return film;
  }

  void write(const std::string &filename, const Film &film) {
    std::ofstream file(filename);
    Float max = film.max();

    file << "P3" << std::endl;
    file << "# " << filename << std::endl;
    file << "# MAX=" << max << std::endl;
    file << film.getWidth() << " " << film.getHeight() << std::endl;
    file << film.getColorRes() << std::endl;

    DEBUG_CODE({
      std::cout << "[WRITE " << filename << "]:";
      std::cout << " width: " << film.getWidth() << ", height: " << film.getHeight();
      std::cout << ", color_resolution: " << film.getColorRes();
      std::cout << ", max: " << max;
      std::cout << std::endl;
    });

    for (size_t i = 0; i < film.size(); i++) {
      Pixel px = film[i];
      size_t r = px.r * film.getColorRes() / max;
      size_t g = px.g * film.getColorRes() / max;
      size_t b = px.b * film.getColorRes() / max;
      file << r << " " << g << " " << b << " ";
    }

    file.close();
  }
} // namespace image

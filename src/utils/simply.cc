#include "simply.hh"
#include "../ver.hh"
#include <fstream>
#include <algorithm>

// #ifndef NDEBUG
// #define ENUM2STR(x) case x: return #x
// #endif

namespace utils {
namespace simply {
  template <typename T>
  static void peek(std::istream &stream, T &data) {
    auto pos = stream.tellg();
    stream >> data;
    stream.seekg(pos);
  }

  static void readASCIIData(std::istream &stream, std::vector<PLYElement> &elements);
  static void readBinaryData(std::istream &stream, std::vector<PLYElement> &elements, bool readLittleEndian);

  static void swapByteOrder(char buffer[], size_t n);

  std::istream& operator>>(std::istream &stream, PLYFormat &format) {
    std::string word, fmt, v;
    stream >> word >> fmt >> v;

    if (word != "format")
      throw std::runtime_error("Expected word: \"format\" but got: \"" + word + "\"\n");

    if (fmt == "ascii")
      format.format = ASCII;
    else if (fmt == "binary_little_endian")
      format.format = BINARY_LE;
    else if (fmt == "binary_big_endian")
      format.format = BINARY_BE;
    else
      throw std::runtime_error("Format: \"" + fmt + "\" unknown!\n");

    if (v != "1.0")
      throw std::runtime_error("Version: \"" + v + "\" not implemented yet!\n");
    format.version = 1;

    #ifndef NDEBUG
    format.name = fmt;
    #endif

    return stream;
  }

  std::istream& operator>>(std::istream &stream, PLYPropType &type) {
    std::string t;
    stream >> t;

    if      (t == "char")   { type.type = CHAR;   type.size = 1; }
    else if (t == "uchar")  { type.type = UCHAR;  type.size = 1; }
    else if (t == "short")  { type.type = SHORT;  type.size = 2; }
    else if (t == "ushort") { type.type = USHORT; type.size = 2; }
    else if (t == "int")    { type.type = INT;    type.size = 4; }
    else if (t == "uint")   { type.type = UINT;   type.size = 4; }
    else if (t == "float")  { type.type = FLOAT;  type.size = 4; }
    else if (t == "double") { type.type = DOUBLE; type.size = 8; }
    else if (t == "list")   { type.type = LIST;   type.size = 0; }
    else throw std::runtime_error("Property type: \"" + t + "\" unknown!\n");

    #ifndef NDEBUG
    type.name = t;
    #endif

    return stream;
  }

  std::istream& operator>>(std::istream &stream, PLYProperty &property) {
    stream >> property.type;
    property.size = property.type.size;

    if (property.type.type == LIST) {
      stream >> property.ctype >> property.dtype;
      property.size += property.ctype.size + property.dtype.size; // Does not really make sense because we don't know yet the list size
      if (property.ctype.type == LIST)
        throw std::runtime_error("List count can't be a list!");
      if (property.dtype.type == LIST)
        throw std::runtime_error("You can't have a list of lists! (i think)");
    }

    stream >> property.name;

    return stream;
  }

  std::istream& operator>>(std::istream &stream, PLYElement &element) {
    stream >> element.name >> element.amount;

    element.size = 0;

    std::string word;
    bool newElement = false;
    while (!newElement) {
      peek(stream, word);

      if (word == "property") {
        PLYProperty property;
        stream >> word >> property;
        element.size += property.size;
        element.properties.push_back(property);
      }  else {
        newElement = true;
      }
    }

    element.buffer.reserve(element.size * element.amount); // it under-reserves for lists, but we'll make it bigger later

    return stream;
  }

  PLYFile::PLYFile(const std::string &filename) {
    DEBUG_CODE({
      std::cout << "[PLY READ " << filename << "]" << std::endl;
    });

    std::ifstream file(filename, std::ifstream::binary);
    if (!file.is_open())
      throw std::runtime_error("Failed to open file: " + filename + "\n");

    std::string magic;
    file >> magic;

    if (magic != "ply")
      throw std::runtime_error("Bad magic number encontered, expected \"ply\" but got: \"" + magic + "\"\n");

    file >> format;

    std::string word;
    bool eoh = false; // end of header
    while (!eoh) {
      file >> word;
      if (word == "comment") {
        std::getline(file, word);
      } else if (word == "end_header") {
        eoh = true;
      } else if (word == "element") {
        PLYElement element;
        file >> element;
        elements.push_back(element);
      } else {
        throw std::runtime_error("Uknown keyword encontered, expected \"comment\", \"end_header\" or \"element\", but got: \"" + word + "\"\n");
      }
    }

    DEBUG_CODE({
        std::cout << "Format: " << format.name << " v" << format.version << std::endl;
        std::cout << "> Found:" << std::endl;
        for (PLYElement &e : elements) {
          std::cout << "\tElement: " << e.amount << " " << e.name << " with properties: " << std::endl;
          for (PLYProperty &p : e.properties) {
            std::cout << "\t\tProperty: " << p.type.name << " ";
            if (p.type.type == LIST) std::cout << p.ctype.name << " " << p.dtype.name << " ";
            std::cout << p.name << std::endl;
          }
        }

      });

    switch (format.format) {
      case ASCII:     readASCIIData(file, elements);         break;
      case BINARY_LE: readBinaryData(file, elements, true);  break;
      case BINARY_BE: readBinaryData(file, elements, false); break;
    }

    // std::cout << "curr:" << file.tellg() << std::endl;
    // file.seekg(0, file.end);
    // std::cout << "end:" << file.tellg() << std::endl;

    if (!file.eof())
      throw std::runtime_error("Error reading ply file: \"" + filename + "\": did not read untill the end!\n");

    file.close();

    #if 1
    std::string sep = " ";

    std::cout << "ply" << std::endl;
    std::cout << "format ascii 1.0" << std::endl;
    for (PLYElement &e : elements) {
      std::cout << "element " << e.name << " " << e.amount << std::endl;
      for(PLYProperty &p : e.properties) {
        if (p.type.type == LIST)
          std::cout << "property " << p.type.name << " " << p.ctype.name << " " << p.dtype.name << " " << p.name << std::endl;
        else
          std::cout << "property " << p.type.name << " " << p.name << std::endl;
      }
    }
    std::cout << "end_header" << std::endl;
    for (PLYElement &e : elements) {
      size_t j = 0;
      for (size_t i = 0; i < e.amount; i++) {
        for (PLYProperty &p : e.properties) {
          bool isList = p.type.type == LIST;
          ply_property_type type = (isList) ? p.ctype.type : p.type.type;

          ssize_t length;

          switch (type) {
            case CHAR:   std::cout << +e.take<int8_t>(j) << sep; length = (ssize_t)+e.take<int8_t>(j); break;
            case UCHAR:  std::cout << +e.take<uint8_t>(j)<< sep; length = (ssize_t)+e.take<uint8_t>(j);break;
            case SHORT:  std::cout << e.take<int16_t>(j) << sep; length = (ssize_t)e.take<int16_t>(j); break;
            case USHORT: std::cout << e.take<uint16_t>(j)<< sep; length = (ssize_t)e.take<uint16_t>(j);break;
            case INT:    std::cout << e.take<int32_t>(j) << sep; length = (ssize_t)e.take<int32_t>(j); break;
            case UINT:   std::cout << e.take<uint32_t>(j)<< sep; length = (ssize_t)e.take<uint32_t>(j);break;
            case FLOAT:  std::cout << e.take<float>(j)   << sep; length = (ssize_t)e.take<float>(j);   break;
            case DOUBLE: std::cout << e.take<double>(j)  << sep; length = (ssize_t)e.take<double>(j);  break;
            case LIST:   break;
          }
          j += p.type.size; // 0 list

          if (isList) {
            j += p.ctype.size;
            for (ssize_t k = 0; k < length; k++) {
              switch (p.dtype.type) {
                case CHAR:   std::cout << +e.take<int8_t>(j) << sep; break;
                case UCHAR:  std::cout << +e.take<uint8_t>(j)<< sep; break;
                case SHORT:  std::cout << e.take<int16_t>(j) << sep; break;
                case USHORT: std::cout << e.take<uint16_t>(j)<< sep; break;
                case INT:    std::cout << e.take<int32_t>(j) << sep; break;
                case UINT:   std::cout << e.take<uint32_t>(j)<< sep; break;
                case FLOAT:  std::cout << e.take<float>(j)   << sep; break;
                case DOUBLE: std::cout << e.take<double>(j)  << sep; break;
                case LIST: break;
              }
              j += p.dtype.size;
            }
          }
        }
        std::cout << std::endl;
      }
    }
    #endif
  }

  void readASCIIData(std::istream &stream, std::vector<PLYElement> &elements) {
    for (PLYElement &e : elements) {
      DEBUG_CODE({
          std::cout << "> Reading(ASCII) " << e.name << " data..." << std::endl;
        });

      for (size_t i = 0; i < e.amount; i++) {
        ssize_t length = 0;
        for (PLYProperty &p : e.properties) {
          bool isList = p.type.type == LIST;
          ply_property_type type = (isList) ? p.ctype.type : p.type.type;

          std::string word;
          stream >> word;

          // switch (type) {
          //   case CHAR:  {int8_t   data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case UCHAR: {uint8_t  data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case SHORT: {int16_t  data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case USHORT:{uint16_t data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case INT:   {int32_t  data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case UINT:  {uint32_t data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case FLOAT: {float    data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case DOUBLE:{double   data = 0; stream >> data; e.write(data); length = (ssize_t)data; } break;
          //   case LIST: break;
          // }

          switch (type) {
            case CHAR:  e.write((int8_t)std::stoi(word));  length = (ssize_t)std::stoi(word); break;
            case UCHAR: e.write((uint8_t)std::stoi(word)); length = (ssize_t)std::stoi(word); break;
            case SHORT: e.write((int16_t)std::stoi(word)); length = (ssize_t)std::stoi(word); break;
            case USHORT:e.write((uint16_t)std::stoi(word));length = (ssize_t)std::stoi(word); break;
            case INT:   e.write((int32_t)std::stoi(word)); length = (ssize_t)std::stoi(word); break;
            case UINT:  e.write((uint32_t)std::stoi(word));length = (ssize_t)std::stoi(word); break;
            case FLOAT: e.write((float)std::stof(word));   length = (ssize_t)std::stof(word); break;
            case DOUBLE:e.write((double)std::stod(word));  length = (ssize_t)std::stod(word); break;
            case LIST: break;
          }

          if (isList) {
            // std::cout << ">" << length << std::endl;

            if (length <= 0)
                throw std::runtime_error("Unexpected error: pls contanct me! (hope this never happens)\n");

            for (ssize_t j = 0; j < length; j++) {
              switch (p.dtype.type) {
                case CHAR:  {int8_t   data; stream >> data; e.write(data);} break;
                case UCHAR: {uint8_t  data; stream >> data; e.write(data);} break;
                case SHORT: {int16_t  data; stream >> data; e.write(data);} break;
                case USHORT:{uint16_t data; stream >> data; e.write(data);} break;
                case INT:   {int32_t  data; stream >> data; e.write(data);} break;
                case UINT:  {uint32_t data; stream >> data; e.write(data);} break;
                case FLOAT: {float    data; stream >> data; e.write(data);} break;
                case DOUBLE:{double   data; stream >> data; e.write(data);} break;
                case LIST: break;
              }
            }
          }
        }
      }
    }
    // char wtf;
    // stream.read(&wtf, 1); // Guess what

    int wtf;
    stream >> wtf;
    std::cout << "WTF: " << wtf << std::endl;
  }

  void readBinaryData(std::istream &stream, std::vector<PLYElement> &elements, bool readLittleEndian) {
    union {
      char            buffer[8];
      unsigned char  ubuffer[8];
      int16_t   short_buffer[4];
      uint16_t ushort_buffer[4];
      int32_t     int_buffer[2];
      uint32_t   uint_buffer[2];
      float     float_buffer[2];
      double   double_buffer;
    } buf;

    stream.read(buf.buffer, 1); // TODO: Why? (this "bug" made me lose like 2h btw)
    for (PLYElement &e : elements) {
      DEBUG_CODE({
          std::cout << "> Reading " << (readLittleEndian ? "(Little Endian)" : "(Big Endian)") << " " << e.name << " data..." << std::endl;
        });

      bool firstPass = true;
      for (size_t i = 0; i < e.amount; i++) {
        for (PLYProperty &p : e.properties) {
          size_t siz = (p.type.type != LIST) ? p.type.size : p.ctype.size;

          stream.read(buf.buffer, siz);

          #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
          if (readLittleEndian)
            swapByteOrder(buf.buffer, siz);
          #else
          if (!readLittleEndian)
            swapByteOrder(buf.buffer, siz);
          #endif

          for (size_t j = 0; j < siz; j++)
            e.buffer.push_back(buf.buffer[j]);

          if (p.type.type == LIST) {
            ssize_t length = 0;
            switch (p.ctype.type) {
              case CHAR:  length = (ssize_t)buf.buffer[0];       break;
              case UCHAR: length = (ssize_t)buf.ubuffer[0];      break;
              case SHORT: length = (ssize_t)buf.short_buffer[0]; break;
              case USHORT:length = (ssize_t)buf.ushort_buffer[0];break;
              case INT:   length = (ssize_t)buf.int_buffer[0];   break;
              case UINT:  length = (ssize_t)buf.uint_buffer[0];  break;
              case FLOAT: length = (ssize_t)buf.float_buffer[0]; break;
              case DOUBLE:length = (ssize_t)buf.double_buffer;   break;
              case LIST:  break;
            }

            if (length <= 0)
                throw std::runtime_error("Unexpected error: pls contanct me! (hope this never happens)\n");

            if (firstPass)
              e.buffer.reserve((length * p.dtype.size + p.ctype.size) * e.amount);
            
            siz = p.dtype.size;
            for (ssize_t k = 0; k < length; k++) {
              stream.read(buf.buffer, siz);

              #if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
              if (readLittleEndian)
                swapByteOrder(buf.buffer, siz);
              #else
              if (!readLittleEndian)
                swapByteOrder(buf.buffer, siz);
              #endif

              for (size_t j = 0; j < siz; j++)
                e.buffer.push_back(buf.buffer[j]);

            }
          }
          firstPass = false;
        }
      }
    }
    stream.read(buf.buffer, 1); // TODO: Why? (this "bug" made me lose like 2h btw)
  }

  bool PLYFile::getPropertyIndexer(const std::string &property_name, PLYIndexer &indexer) const {
    for (size_t i = 0; i < elements.size(); i++) {
      const auto &e = elements[i];
      size_t start = 0;
      for (const auto &p : e.properties) {
        if (p.name == property_name) {
          indexer.element_idx = i;
          indexer.start = start;
          indexer.offset = e.size;
          indexer.type = p.type;

          indexer.ctype = p.ctype;
          indexer.dtype = p.dtype;

          indexer.amount = e.amount;

          return true;
        }
        start += p.type.size;
        if (p.type.type == LIST)
          start += p.ctype.size + p.dtype.size;
      }
    }
    return false;
  }

  void swapByteOrder(char buffer[], size_t n) {
    for (size_t i = 0; i < n / 2; i++)
      std::swap(buffer[i], buffer[n - 1 - i]); 
  }
} // namespace simply
} // namespace utils

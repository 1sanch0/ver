#ifndef SIMPLY_H_
#define SIMPLY_H_

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <stddef.h>

namespace utils {
  // very SIMPLE PLY reader
  namespace simply {
    enum ply_format {
      ASCII,
      BINARY_LE,
      BINARY_BE
    };

    enum ply_property_type {
      CHAR,
      UCHAR,
      SHORT,
      USHORT,
      INT,
      UINT,
      FLOAT,
      DOUBLE,

      LIST
    };

    struct PLYFormat {
      ply_format format;
      int version;

      // #ifndef NDEBUG
      std::string name;
      // #endif

      friend std::istream& operator>>(std::istream &stream, PLYFormat &format);
    };

    struct PLYPropType {
      ply_property_type type;
      size_t size;

      //#ifndef NDEBUG
      std::string name;
      //#endif

      friend std::istream& operator>>(std::istream &stream, PLYPropType &type);
    };

    struct PLYProperty {
      std::string name;
      PLYPropType type;
      size_t size;

      // in case a list is used
      PLYPropType ctype, dtype;

      friend std::istream& operator>>(std::istream &stream, PLYProperty &property);
    };

    struct PLYElement {
      std::string name;
      std::vector<PLYProperty> properties;
      size_t size; // in bytes
      size_t amount;

      std::vector<char> buffer; // all the data of the element will be serialized here

      friend std::istream& operator>>(std::istream &stream, PLYElement &element);

      template<typename T>
      T take(size_t idx) const {
        if (idx + sizeof(T) > buffer.size())
          throw std::runtime_error("Can't take " + std::to_string(sizeof(T)) + " bytes from buffer! (overflowed)\n");

        // Assuming endianness of the buffer data and architecture matches
        T value;
        std::memcpy(&value, &buffer[idx], sizeof(T));
        return value;
      }

      template<typename T>
      void write(T value) {
        char *buf = reinterpret_cast<char *>(&value);
        for (size_t i = 0; i < sizeof(T); i++)
          buffer.push_back(buf[i]);
      }
    };

    struct PLYIndexer {
      size_t element_idx;
      size_t start;
      size_t offset;
      PLYPropType type;
      PLYPropType ctype, dtype; // List
      size_t amount;
    };

    struct PLYFile {
      PLYFormat format;
      std::vector<PLYElement> elements;

      explicit PLYFile(const std::string &filename);

      bool getPropertyIndexer(const std::string &property_name, PLYIndexer &indexer) const;

    };

  } // namespace simply
}

#endif // SIMPLY_H_

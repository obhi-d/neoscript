#include <cstdint>
#include <string>

namespace neo {

struct position {
  std::uint32_t line        = 0;
  std::uint32_t character   = 0;
  friend bool   operator==(position const& first, position const& second) {
    return 
           first.line == second.line && first.character == second.character;
  }
  friend bool operator!=(position const& first, position const& second) {
    return !(first == second);
  }

};

struct location {

  void step() { begin = end; }
  void columns(std::uint32_t l) { end.character += l; }
  void lines(std::uint32_t l) { end.character += l; }

  bool operator==(location const& p) const {
    return p.begin == begin && p.end == end;
  }
  bool operator!=(location const& p) const { return !(*this == p); }

  inline operator std::string() const {
    std::string value = source_name ? *source_name : "buffer";
    value += "(" + std::to_string(begin.line) + ":" +
             std::to_string(begin.character) + "-" + std::to_string(end.line) +
             ":" + std::to_string(end.character) + ")";
    return value;
  }

  friend std::ostream& operator<<(std::ostream& yyo, location const& l) {
    std::string value = source_name ? *source_name : "buffer";
    if (begin == end)
      yyo << "<" << value << '-' << l.begin << ">";
    else
      yyo << "<" << value << '-' << l.begin << "-" << l.end << ">";
    return yyo;
  }

  std::string const* source_name = nullptr;
  position begin;
  position end;
};

} // namespace neo
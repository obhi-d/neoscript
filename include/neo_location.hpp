#pragma once
#include <cstdint>
#include <neo_common.hpp>
#include <ostream>
#include <string>

namespace neo
{

struct NEO_API position
{
  std::uint32_t line      = 1;
  std::uint32_t character = 1;
  friend bool operator==(position const& first, position const& second) noexcept
  {
    return first.line == second.line && first.character == second.character;
  }
  friend bool operator!=(position const& first, position const& second) noexcept
  {
    return !(first == second);
  }

  friend std::ostream& operator<<(std::ostream& yyo, position const& l) noexcept
  {
    yyo << l.line << ':' << l.character;
    return yyo;
  }
};

class NEO_API location
{
public:
  void step() noexcept { begin = end; }
  void columns(std::uint32_t l) noexcept { end.character += l; }
  void lines(std::uint32_t l) noexcept
  {
    end.line += l;
    end.character = 0;
  }

  bool operator==(location const& p) const noexcept
  {
    return p.begin == begin && p.end == end;
  }
  bool operator!=(location const& p) const noexcept { return !(*this == p); }

  inline operator std::string() const noexcept

  {
    std::string value = std::string(source_name.empty() ? "buffer" : source_name);
    value += "(" + std::to_string(begin.line) + ":" +
             std::to_string(begin.character) + "-" + std::to_string(end.line) +
             ":" + std::to_string(end.character) + "): ";
    return value;
  }

  friend std::ostream& operator<<(std::ostream& yyo, location const& l) noexcept

  {
    std::string value =
        std::string(l.source_name.empty() ? "buffer" : l.source_name);
    if (l.begin == l.end)
      yyo << "<" << value << '-' << l.begin << ">";
    else
      yyo << "<" << value << '-' << l.begin << "-" << l.end << ">";
    return yyo;
  }

  std::string_view   source_name;
  position           begin;
  position           end;
};

} // namespace neo
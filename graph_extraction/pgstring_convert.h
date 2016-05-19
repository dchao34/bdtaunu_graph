#ifndef _PGSTRING_CONVERT_H_
#define _PGSTRING_CONVERT_H_

#include <string>
#include <vector>
#include <algorithm>
#include <boost/tokenizer.hpp>

// functions that convert between postgres text data 
// and the binary type. 

// helper trait class and specializations
template <typename T>
class pgstring_conversion_traits;

template <>
class pgstring_conversion_traits<int> {
  public:
    static int convert(const std::string &s) { return std::stoi(s); }
};

template <>
class pgstring_conversion_traits<float> {
  public:
    static float convert(const std::string &s) { return std::stof(s); }
};

template <>
class pgstring_conversion_traits<double> {
  public:
    static double convert(const std::string &s) { return std::stod(s); }
};


// functions that convert postgres text data to fundamental types 
inline void pgstring_convert(const std::string &s, int &v) { v = std::stoi(s); }
inline void pgstring_convert(const std::string &s, float &v) { v = std::stof(s); }
inline void pgstring_convert(const std::string &s, double &v) { v = std::stod(s); }

// function that convert postgres text data to std::vector
template <typename T> 
void pgstring_convert(
    const std::string &s, std::vector<T> &v, 
    const std::string &enclosure_chars="{}",
    const boost::char_separator<char> &sep = 
    boost::char_separator<char>(",", "", boost::keep_empty_tokens)) {

  v.clear();

  size_t first_idx = s.find_first_not_of(enclosure_chars);
  size_t last_idx = s.find_last_not_of(enclosure_chars);
  if (first_idx == std::string::npos || last_idx == std::string::npos) { return; }

  boost::tokenizer<boost::char_separator<char>> tok(
      s.begin()+first_idx, s.begin()+last_idx+1, sep);

  transform(tok.begin(), tok.end(), 
            std::back_inserter(v), 
            pgstring_conversion_traits<T>::convert);
}

// function that converts std::vector to postgres text data. 
// NOTE: uses std::to_string(); this may not be what you want for float types. 
template <typename T> 
std::string vector2pgstring(const std::vector<T> &v) {

  if (v.empty()) { return "{}"; }

  std::string s = "\"{";

  for (const auto &e : v) {
    s += std::to_string(e) + ",";
  }

  s.pop_back(); s += "}\"";
  return s;
}

#endif

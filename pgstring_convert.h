#include <string>
#include <vector>
#include <algorithm>
#include <boost/tokenizer.hpp>

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



inline void pgstring_convert(const std::string &s, int &v) { v = std::stoi(s); }
inline void pgstring_convert(const std::string &s, float &v) { v = std::stof(s); }
inline void pgstring_convert(const std::string &s, double &v) { v = std::stod(s); }

template <typename T> 
void pgstring_convert(
    const std::string &s, std::vector<T> &v, 
    const std::string &enclosure_chars="{}",
    const boost::char_separator<char> &sep = 
    boost::char_separator<char>(",", "", boost::keep_empty_tokens)) {

  boost::tokenizer<boost::char_separator<char>> tok(
      s.begin()+s.find_first_not_of(enclosure_chars),
      s.begin()+s.find_last_not_of(enclosure_chars)+1,
      sep);

  v.clear();
  transform(tok.begin(), tok.end(), 
            std::back_inserter(v), 
            pgstring_conversion_traits<T>::convert);
}


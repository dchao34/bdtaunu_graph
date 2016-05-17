template <typename TokenizerFunction>
void CsvReader<TokenizerFunction>::open(const std::string &fname) {

  // check if another file is already open.
  if (fin_.is_open()) {
    throw std::runtime_error(
        "CsvReader<>::open() : another file is already open. " 
        "run close() before running another open(). "
    );
  }
  assert(colname_idx_.empty());
  assert(cache_.empty());

  // open the file and read the title line
  fin_.open(fname);
  if (!fin_.is_open()) {
    throw std::runtime_error(
      "CsvReader<>::open() : file " + fname + " does not exist. " 
    );
  }
  std::getline(fin_, line_);

  // assign column name indices and caches
  TokenIterator beg = 
    boost::make_token_iterator<std::string>(line_.begin(), line_.end(), sep_), end;
  for (size_t idx = 0; beg != end; ++beg, ++idx) { 
    colname_idx_[*beg] = idx;
    cache_.push_back("");
  }
}

template <typename TokenizerFunction>
void CsvReader<TokenizerFunction>::close() {
  fin_.close(); 
  colname_idx_.clear(); 
  cache_.clear(); 
}

template <typename TokenizerFunction>
bool CsvReader<TokenizerFunction>::next() {

  // read the next line 
  if (!getline(fin_, line_)) { return false; }

  // construct tokenizer
  TokenIterator beg = 
    boost::make_token_iterator<std::string>(line_.begin(), line_.end(), sep_);
  TokenIterator end;

  // store each entry into the cache
  for (size_t i = 0; beg != end; ++beg, ++i) { cache_[i] = *beg;}

  return true;

}


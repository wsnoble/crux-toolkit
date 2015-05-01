#include "FileUtils.h"

#include "boost/filesystem.hpp"

#include <fstream>
#include <stdexcept>

using namespace std;

bool FileUtils::Exists(const string& path) {
  return boost::filesystem::exists(path);
}

void FileUtils::Rename(const string& from, const string& to) {
  if (Exists(from)) {
    boost::filesystem::rename(from, to);
  }
}

void FileUtils::Remove(const string& path) {
  if (Exists(path)) {
    boost::filesystem::remove_all(path);
  }
}

string FileUtils::Read(const string& path) {
  try {
    ifstream stream(path.c_str());
    stream.seekg(0, ios::end);
    size_t size = stream.tellg();
    string buffer(size, '\0');
    stream.seekg(0);
    stream.read(&buffer[0], size);
    return buffer;
  } catch (...) {
    throw runtime_error("Error reading file '" + path + "'");
  }
}

ofstream* FileUtils::GetWriteStream(const string& path, bool overwrite) {
  if (Exists(path) && !overwrite) {
    return NULL;
  }
  ofstream* stream = new ofstream(path.c_str());
  if (!stream->good()) {
    delete stream;
    return NULL;
  }
  return stream;
}

string FileUtils::BaseName(const string& path) {
  boost::filesystem::path p(path);
  return p.has_filename() ? p.filename().string() : "";
}

string FileUtils::DirName(const string& path) {
  boost::filesystem::path p(path);
  return p.has_parent_path() ? p.parent_path().string() : "";
}

string FileUtils::Stem(const string& path) {
  boost::filesystem::path p(path);
  return p.has_stem() ? p.stem().string() : "";
}

string FileUtils::Extension(const string& path) {
  boost::filesystem::path p(path);
  return p.has_extension() ? p.extension().string() : "";
}


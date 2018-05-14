#pragma once

#include "Container.h"
#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace dd_fs = std::experimental::filesystem;
typedef std::experimental::filesystem::directory_iterator dd_fs_dir;

enum ddIOflag { READ = 0x1, WRITE = 0x2, APPEND = 0x4, DIRECTORY = 0x8 };

template<unsigned T = 1024>
class ddFileIO {
public:
  /** \brief Destructor cleans up */
  ~ddFileIO() {
    if (file_handle.is_open()) file_handle.close();
  }

  /** \brief Opens a file or directory w/ flags */
  bool open(const char *fileName, const ddIOflag flags) {
    std::ios_base::openmode ios_flag = std::ios::in;

    if ((unsigned)(flags & ddIOflag::READ)) {
      // open simple file to read from
      ios_flag = std::ios::in;
    } else if ((unsigned)(flags & ddIOflag::WRITE)) {
      // create & open file for writting to
      ios_flag = std::ios::out;
    } else if ((unsigned)(flags & ddIOflag::APPEND)) {
      // create/open file for writting to end of 
      ios_flag = std::ios::app;
    } else if ((unsigned)(flags & ddIOflag::DIRECTORY)) {
      // opens directory and records all files in it
      dd_fs_dir directory = dd_fs_dir(fileName);
      if (directory != dd_fs::end(directory)) {
        dir_files = std::move(parse_directory2(directory));
        return true;
      }
      return false;
    }

    file_handle.open(fileName, ios_flag);
    return file_handle.good();
  }

  /** \brief Return last string read in */
  const char *readNextLine() {
    if (!file_handle.eof()) {
      file_handle.getline(line, T);
      if (*line) return line;
    }
    return nullptr;
  }

  /** \brief Write a line to an already opened file */
  void writeLine(const char *output) {
    if (file_handle.good()) {
      file_handle << output;
    }
  }

  /** \brief Return array of file in current opened directory*/
  inline const dd_array<std::string> get_directory_files() { return dir_files; }

private:
  char line[T];
  std::fstream file_handle;
  dd_array<std::string> dir_files;

  dd_array<std::string> parse_directory2(dd_fs_dir &dir_handle) {
    dd_array<std::string> files;
    std::vector<std::string> vec_files;
    // count the number of files
    unsigned num_files = 0;
    for (auto &p : dd_fs::directory_iterator(dir_handle)) {
      vec_files.push_back(p.path().string());
      num_files++;
    }

    files.resize(num_files);
    // need to sort vector since directory is not sorted on all filesystems
    std::sort(vec_files.begin(), vec_files.end());

    unsigned file_idx = 0;
    for (auto &p : vec_files) {
      files[file_idx] = p;
      file_idx++;
    }

    return files;
  }
};
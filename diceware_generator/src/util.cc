#include "util.h"
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

namespace fs = std::filesystem;

std::string util::GetShareDir() {
  std::string exe_path;
  fs::path p{"/proc/self/exe"};

  if (fs::exists(p) && fs::is_symlink(p)) {
    exe_path = fs::read_symlink(p);
  } else {
    // Default path, ideally shouldn't come here.
    exe_path = "/usr/bin/executable";
  }

  auto exe_dir = exe_path.substr(0, exe_path.rfind('/'));
  exe_dir = exe_dir.substr(0, exe_dir.rfind('/'));
  return exe_dir + "/share/diceware_generator";
}

std::string util::GetConfigDir() {
  std::string home_dir;
  home_dir = getenv("HOME");
  if (home_dir.empty()) {
    home_dir = getpwuid(getuid())->pw_dir;
  }
  auto config_dir = home_dir + "/.config/diceware_generator";
  std::filesystem::create_directories(config_dir);
  return config_dir;
}

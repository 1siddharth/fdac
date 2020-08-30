#pragma once

#include <cassert>
#include <stdexcept>
#include <type_traits>
#include <errno.h>
#include <fcntl.h>
#include <linux/kcmp.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef __NR_kcmp
#error "KCMP missing"
#endif

namespace fdcap {

constexpr inline dev_t FDCAP_DEV_MAX = static_cast<dev_t>(-1);
constexpr inline ino_t FDCAP_INO_MAX = static_cast<ino_t>(-1);

class FDCap {
private:
  int fd = -1;
  mutable dev_t dev = FDCAP_DEV_MAX;
  mutable ino_t ino = FDCAP_INO_MAX;

  static void lazy_init(const FDCap& f)
  {
    if (f.dev == FDCAP_DEV_MAX)
      lazy_init_dev_ino(f);
  }
  static void lazy_init_dev_ino(const FDCap& f)
  {
    assert(f.ino == FDCAP_INO_MAX);
    assert(f.fd > -1);
    struct stat st;
    int r = fstat(f.fd, &st);
    if (r < 0)
      throw std::runtime_error("Failed to fstat file descriptor");
    f.dev = st.st_dev;
    f.ino = st.st_ino;
  }
  void move_from(const FDCap& f)
  {
    close(fd);
    fd = fcntl(f.fd, F_DUPFD_CLOEXEC, 3);
    if (fd < 0)
      throw std::runtime_error("Failed to duplicate descriptor");
    dev = f.dev;
    ino = f.ino;
  }
  void move_from(FDCap&& f)
  {
    close(fd);
    fd = f.fd;
    dev = f.dev;
    ino = f.ino;
    f.fd = -1;
    f.dev = FDCAP_DEV_MAX;
    f.ino = FDCAP_INO_MAX;
  }
public:
  template <typename... List,
	    typename = std::enable_if_t<std::conjunction_v<std::is_same<std::remove_cvref_t<List>, FDCap>...>>>
  static void lazy_init(const FDCap& f, const List&... list)
  {
    if (f.dev == FDCAP_DEV_MAX)
      lazy_init(f);
    if (sizeof...(list) > 0)
      lazy_init(list...);
  }
  static bool is_same_file(const FDCap& a, const FDCap& b)
  {
    if (a.fd < 0 || b.fd < 0)
      return false;
    pid_t pid = getpid();
    return !syscall(__NR_kcmp, pid, pid, KCMP_FILE, a.get(), b.get());
  }
  static bool is_same_file_object(const FDCap& a, const FDCap& b)
  {
    lazy_init(a, b);
    if ((a.dev == b.dev) && (a.ino == b.ino))
      return true;
    return false;
  }

  explicit FDCap(int fd_) : fd(fd_)
  {
    if (fd < 0)
      throw std::runtime_error("Bad File Descriptor");
  }
  FDCap(const FDCap& f)
  {
    move_from(f);
  }
  FDCap& operator=(FDCap& f)
  {
    move_from(f);
    return *this;
  }
  FDCap(FDCap&& f)
  {
    move_from(std::move(f));
  }
  FDCap& operator=(FDCap&& f)
  {
    move_from(std::move(f));
    return *this;
  }
  ~FDCap()
  {
    close(fd);
  }
  bool operator==(const FDCap& f) const
  {
    return is_same_file(*this, f);
  }
  int get() const
  {
    return fd;
  }
  void reset(int fd_)
  {
    close(fd);
    fd = fd_;
    dev = FDCAP_DEV_MAX;
    ino = FDCAP_INO_MAX;
  }
};

} // namespace fdcap

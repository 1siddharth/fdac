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

class FDCap;

template <typename T>
concept OnlyFDCap = requires { std::is_same_v<std::remove_cvref_t<T>, FDCap> == true; };

class FDCap {
private:
  int fd = -1;
  mutable dev_t dev = -1;
  mutable ino_t ino = -1;

  template <OnlyFDCap... List>
  static void lazy_init(const FDCap& f, const List&... list)
  {
    if (f.dev == static_cast<dev_t>(-1))
      lazy_init(f);
    if (sizeof...(list) > 0)
      lazy_init(list...);
  }
  static void lazy_init(const FDCap& f)
  {
    if (f.dev == static_cast<dev_t>(-1))
      lazy_init_dev_ino(f);
  }
  static void lazy_init_dev_ino(const FDCap& f)
  {
    assert(f.ino == static_cast<ino_t>(-1));
    struct stat st;
    int r = fstat(f.fd, &st);
    if (r < 0) {
      throw std::runtime_error("Failed to fstat fd");
    }
    f.dev = st.st_dev;
    f.ino = st.st_ino;
  }
  void move_from(const FDCap& f)
  {
    close(fd);
    fd = fcntl(fd, F_DUPFD_CLOEXEC);
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
  }
public:
  static bool is_same_file(const FDCap& a, const FDCap& b)
  {
    pid_t pid = getpid();
    int r = syscall(__NR_kcmp, pid, pid, KCMP_FILE, a.get(), b.get());
    if (__builtin_expect(r < 0, 0))
      return false;
    return !!r;
  }
  static bool is_same_file_object(const FDCap& a, const FDCap& b)
  {
    lazy_init(a, b);
    if ((a.dev == b.dev) && (a.ino == b.ino))
      return true;
    return false;
  }

  explicit FDCap(int fd_) : fd(fd_) {}
  FDCap(const FDCap& f)
  {
    move_from(f);
  }
  FDCap& operator=(const FDCap& f)
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
};

} // namespace fdcap

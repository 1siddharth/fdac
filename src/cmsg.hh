#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

inline ssize_t FdWrite(int sockfd, void *buf, size_t buflen, int fd) {
  struct msghdr msg;
  struct iovec iov;
  union {
    struct cmsghdr cmsghdr;
    char control[CMSG_SPACE(sizeof(int))];
  } cmsg_un;
  struct cmsghdr *cmsg;

  iov.iov_base = buf;
  iov.iov_len = buflen;

  msg.msg_name = nullptr;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;

  if (fd != -1) {
    msg.msg_control = cmsg_un.control;
    msg.msg_controllen = sizeof(cmsg_un.control);

    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    std::cerr << "Passing fd " << fd << '\n';
    *(reinterpret_cast<int *>(CMSG_DATA(cmsg))) = fd;
  } else {
    msg.msg_control = nullptr;
    msg.msg_controllen = 0;
    std::cerr << "Not passing fd" << '\n';
  }

  return sendmsg(sockfd, &msg, 0);
}

inline ssize_t FdRead(int sockfd, void *buf, size_t bufsize, int *fd) {
  if (fd) {
    struct msghdr msg;
    struct iovec iov;
    union {
      struct cmsghdr cmsghdr;
      char control[CMSG_SPACE(sizeof(int))];
    } cmsg_un;
    struct cmsghdr *cmsg;

    iov.iov_base = buf;
    iov.iov_len = bufsize;

    msg.msg_name = nullptr;
    msg.msg_namelen = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_un.control;
    msg.msg_controllen = sizeof(cmsg_un.control);
    auto r = recvmsg(sockfd, &msg, 0);
    if (r < 0) {
      return r;
    }
    cmsg = CMSG_FIRSTHDR(&msg);
    if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int))) {
      if (cmsg->cmsg_level != SOL_SOCKET) {
        std::cerr << "Invalid cmsg_level " << cmsg->cmsg_level << '\n';
        return -1;
      }
      if (cmsg->cmsg_type != SCM_RIGHTS) {
        std::cerr << "Invalid cmsg_type " << cmsg->cmsg_type << '\n';
        return -1;
      }

      *fd = *(reinterpret_cast<int *>(CMSG_DATA(cmsg)));
      std::cerr << "Received fd " << *fd << '\n';
      return r;
    } else
      *fd = -1;
  } else {
    return read(sockfd, buf, bufsize);
  }
  return -1;
}

#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cmsg.hh>
#include <fdcache.hh>
#include <fdcap.hh>

// This shows how fds passed down during fork from the parent to the child will
// compare equal, as they will internally point to the same open file.

TEST(Inheritance, Basic1) {
  int fd[2];
  EXPECT_TRUE(!socketpair(AF_UNIX, SOCK_STREAM, 0, fd));
  fdcap::FDCap f1{0};
  auto pid = fork();
  if (pid) {
    int new_fd;
    char buf[2];
    EXPECT_GE(FdRead(fd[0], buf, 2, &new_fd), 0);
    ASSERT_EQ(fdcap::FDCap{new_fd}, f1);
  } else {
    char buf[2] = "x";
    EXPECT_GE(FdWrite(fd[1], buf, 2, 0), 0);
  }
}

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

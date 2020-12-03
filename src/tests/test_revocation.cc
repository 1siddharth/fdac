#include <gtest/gtest.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cmsg.hh>
#include <fdcache.hh>
#include <fdcap.hh>
#include <string>

FDCache<std::string> fdc;

// This demonstrates revocation. This is fairly simple to understand: we keep
// the fds keyed by a string, and upon reception from the child, we insert and
// compare, then we revoke and replace with another key (which can be done
// directly using replace too), and then the comparison for both fails, as they
// reference distinct open files. Yeah, boring stuff.

TEST(Revocation, Basic1) {
  int fd[2];
  EXPECT_TRUE(!socketpair(AF_UNIX, SOCK_STREAM, 0, fd));
  fdcap::FDCap f1{0};
  auto pid = fork();
  if (pid) {
    int new_fd;
    char buf[2];
    EXPECT_GE(FdRead(fd[0], buf, 2, &new_fd), 0);
    EXPECT_TRUE(fdc.insert(std::string(buf), fdcap::FDCap{new_fd}));
    ASSERT_EQ(*fdc.get("x"), fdcap::FDCap{new_fd});
    EXPECT_GE(FdRead(fd[0], buf, 2, &new_fd), 0);
    fdc.revoke("x");
    ASSERT_FALSE(fdc.get("x"));
    EXPECT_TRUE(fdc.insert("x", fdcap::FDCap{fd[0]}));
    ASSERT_NE(*fdc.get("x"), fdcap::FDCap{new_fd});
  } else {
    char buf[2] = "x";
    EXPECT_GE(FdWrite(fd[1], buf, 2, 0), 0);
    EXPECT_GE(FdWrite(fd[1], buf, 2, 0), 0);
  }
}

int main() {
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

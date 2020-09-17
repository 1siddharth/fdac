#include <iostream>
#include <gtest/gtest.h>
#include <fdcap.hh>

using namespace fdcap;

TEST(FDCap, Invariants1)
{
  int fd[2];
  int r = pipe(fd);
  if (r < 0)
    throw std::runtime_error("Failed to create pipes");
  FDCap A(dup3(0, 5, 0));
  FDCap B(fd[0]);
  int rr = fcntl(B.get(), F_DUPFD_CLOEXEC, 3);
  if (rr < 0)
    throw std::runtime_error("FCNTL ERROR");
  FDCap C(fd[1]);
  ASSERT_FALSE(FDCap::is_same_file(A, B));
  ASSERT_FALSE(A == B);
  ASSERT_FALSE(FDCap::is_same_file(B, C));
  ASSERT_FALSE(B == C);
  ASSERT_FALSE(FDCap::is_same_file(C, A));
  ASSERT_FALSE(C == A);
  ASSERT_FALSE(FDCap::is_same_file_object(A, B));
  ASSERT_TRUE(FDCap::is_same_file_object(B, C));
  ASSERT_FALSE(FDCap::is_same_file_object(C, A));
  ASSERT_EQ(A.get(), 5);
  ASSERT_EQ(B.get(), fd[0]);
  ASSERT_EQ(C.get(), fd[1]);
  ASSERT_THROW(FDCap(-1), std::runtime_error);
}

TEST(FDCap, Copy1)
{
  FDCap A(dup3(0, 3, 0));
  FDCap B = A;
  ASSERT_EQ(B.get(), 4);
  ASSERT_EQ(A.get(), 3);
  ASSERT_TRUE(FDCap::is_same_file(A, B));
  ASSERT_TRUE(FDCap::is_same_file_object(A, B));
}

TEST(FDCap, Move1)
{
  FDCap A(dup3(0, 3, 0));
  FDCap B(std::move(A));
  ASSERT_EQ(A.get(), -1);
  ASSERT_EQ(B.get(), 3);
  ASSERT_FALSE(FDCap::is_same_file(A, B));
  ASSERT_EQ((A = B).get(), 4);
  ASSERT_EQ(B.get(), 3);
  ASSERT_EQ(A.get(), 4);
  ASSERT_TRUE(FDCap::is_same_file_object(A, B));
}

TEST(FDCap, SelfCopy1)
{
  FDCap A(dup3(0, 3, 0));
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wself-assign-overloaded"
  A = A;
  #pragma GCC diagnostic pop
  ASSERT_EQ(A.get(), 3);
}

TEST(FDCap, SelfMove1)
{
  FDCap A(dup3(0, 3, 0));
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wself-move"
  A = std::move(A);
  #pragma GCC diagnostic pop
  ASSERT_EQ(A.get(), -1);
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

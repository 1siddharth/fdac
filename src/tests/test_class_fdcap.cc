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
  FDCap A(fd[0]);
  FDCap B(1);
  FDCap C(2);
  ASSERT_FALSE(FDCap::is_same_file(A, B));
  ASSERT_FALSE(A == B);
  ASSERT_TRUE(FDCap::is_same_file(B, C));
  ASSERT_TRUE(B == C);
  ASSERT_FALSE(FDCap::is_same_file(C, A));
  ASSERT_FALSE(C == A);
  ASSERT_FALSE(FDCap::is_same_file_object(A, B));
  ASSERT_TRUE(FDCap::is_same_file_object(B, C));
  ASSERT_FALSE(FDCap::is_same_file_object(C, A));
  ASSERT_EQ(A.get(), fd[0]);
  ASSERT_EQ(B.get(), 1);
  ASSERT_EQ(C.get(), 2);
  ASSERT_THROW(FDCap(-1), std::runtime_error);
}

int main()
{
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

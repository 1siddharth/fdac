#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

#include <gtest/gtest.h>
#include <fdcache.hh>

struct S {
  int i = rand();
  size_t diff_type_construct = 0;
  size_t copy_construct = 0;
  size_t copy_assign = 0;
  size_t move_construct = 0;
  size_t move_assign = 0;
  S() = default;
  S(int i_) { i = i_; printf("Constructed from int\n"); }
  S(S& other) { i = other.i; copy_construct++; other.copy_construct++; }
  S& operator=(S& other) { i = other.i; copy_assign++; other.copy_assign++; return *this; }
  S(S&& other) { i = other.i; move_construct++; other.move_construct++; }
  S& operator=(S&& other) { i = other.i; move_assign++; other.move_assign++; return *this; }
  ~S() = default;
  bool operator==(const S& s) const
  {
    return i == s.i;
  }
};

template <>
struct std::hash<S>
{
  size_t operator()(const S& s) const noexcept
  {
    return std::hash<decltype(s.i)>()(s.i);
  }
};

#define print_stats(s)							\
  printf("Current:\ncopy_construct %zu\ncopy_assign %zu\n"		\
         "move_construct %zu\nmove_assign %zu\n",			\
         s.copy_construct, s.copy_assign, s.move_construct, s.move_assign);

#define println(s) printf(s "\n")

#define SETUP()  FDCache<S> fdc; S s; int fd = 3


using fdcap::FDCap;

TEST(FDCache, InsertInvariants1)
{
  SETUP();
  
  println("Insert Key by copy");
  bool b = fdc.insert(s, FDCap(dup3(0, fd++, 0)));
  EXPECT_TRUE(b);
  print_stats(s);
  ASSERT_TRUE(s.copy_construct > 0);

  println("Insert Key by move");
  S s2;
  b = fdc.insert(std::move(s2), FDCap(dup3(0, fd++, 0)));
  EXPECT_TRUE(b);
  print_stats(s2);
  ASSERT_TRUE(s2.move_construct > 0);
}

TEST(FDCache, ReplaceInvariants1)
{
  SETUP();

  bool b = fdc.insert(s, FDCap(dup3(0, fd++, 0)));
  EXPECT_TRUE(b);

  FDCap cap(dup3(0, fd++, 0));
  b = fdc.replace(s, FDCap(dup3(0, fd++, 0)));
  ASSERT_TRUE(b);
  auto p = fdc.get(s);
  ASSERT_EQ(p->get(), fd - 1);

  ASSERT_FALSE(fdc.replace(S{}, FDCap(dup3(0, fd++, 0))));
}

TEST(FDCache, RevokeInvariants1)
{
  SETUP();

  bool b = fdc.insert(s, FDCap(dup3(0, fd++, 0)));
  EXPECT_TRUE(b);

  fdc.revoke(s);
  ASSERT_EQ(fdc.get(s), nullptr);
}

TEST(FDCache, HetKeyInvariants1)
{
  FDCache<std::string_view> fdc;
  int fd = 3;

  EXPECT_TRUE(fdc.insert(std::string_view("foobar"), FDCap(dup3(0, fd++, 0))));
  EXPECT_TRUE(fdc.insert(std::string_view("barfoo"), FDCap(dup3(0, fd++, 0))));
  EXPECT_TRUE(fdc.insert(std::string_view("foobaz"), FDCap(dup3(0, fd++, 0))));

  bool b = fdc.replace(std::string("foobar"), FDCap(dup3(0, fd++, 0)));
  ASSERT_TRUE(b);
  b = fdc.replace("barfoo", FDCap(dup3(0, fd++, 0)));
  ASSERT_TRUE(b);
  std::string tmp = "foobaz";
  b = fdc.replace(tmp, FDCap(dup3(0, fd++, 0)));
  ASSERT_TRUE(b);
}

int main()
{
  testing::InitGoogleTest();
  return RUN_ALL_TESTS();
}

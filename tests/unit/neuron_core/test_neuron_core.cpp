#include "TestHarness.h"

#include <string>
#include <vector>

// Test basic STL and C++23 features that NeuronCore relies on
int main()
{
  io::tests::TestContext ctx("NeuronCoreBasics");

  // Test 1: Basic STL containers work
  {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    IO_EXPECT_TRUE(ctx, vec.size() == 5);
    IO_EXPECT_TRUE(ctx, vec[0] == 1);
  }

  // Test 2: String operations
  {
    std::string str = "NeuronCore";
    IO_EXPECT_TRUE(ctx, !str.empty());
    IO_EXPECT_TRUE(ctx, str.length() == 10);
  }

  // Test 3: C++23 constexpr features
  {
    constexpr int value = 42;
    IO_EXPECT_TRUE(ctx, value == 42);
  }

  // Test 4: Modern C++ ranges (C++20/23)
  {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    int sum = 0;
    for (int n : numbers)
    {
      sum += n;
    }
    IO_EXPECT_TRUE(ctx, sum == 15);
  }

  // Test 5: Smart pointers (RAII)
  {
    auto ptr = std::make_unique<int>(100);
    IO_EXPECT_TRUE(ctx, ptr != nullptr);
    IO_EXPECT_TRUE(ctx, *ptr == 100);
  }

  return ctx.Finalize();
}

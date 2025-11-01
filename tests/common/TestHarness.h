#pragma once

#include <cmath>
#include <iostream>
#include <string_view>

namespace io::tests
{
  class TestContext
  {
    public:
      explicit TestContext(std::string_view suiteName)
        : m_suiteName(suiteName)
      {
        std::cout << "[ RUN      ] " << m_suiteName << '\n';
      }

      ~TestContext()
      {
        if (!m_finished)
          Finalize();
      }

      void ExpectTrue(bool condition, std::string_view message, const char* file, int line)
      {
        if (!condition)
        {
          ++m_failures;
          std::cerr << "[  FAILED  ] " << m_suiteName << ": " << message << " (" << file << ':' << line << ")\n";
        }
      }

      void ExpectNear(double value, double expected, double epsilon, std::string_view message, const char* file, int line)
      {
        ExpectTrue(std::fabs(value - expected) <= epsilon, message, file, line);
      }

      int Finalize()
      {
        m_finished = true;
        if (m_failures == 0)
        {
          std::cout << "[       OK ] " << m_suiteName << '\n';
          return 0;
        }

        std::cerr << "[  FAILED  ] " << m_suiteName << " with " << m_failures << " failure(s).\n";
        return 1;
      }

    private:
      std::string_view m_suiteName;
      int m_failures = 0;
      bool m_finished = false;
  };
}

#define IO_EXPECT_TRUE(ctx, condition) (ctx).ExpectTrue((condition), #condition, __FILE__, __LINE__)
#define IO_EXPECT_NEAR(ctx, value, expected, epsilon) (ctx).ExpectNear((value), (expected), (epsilon), #value " ~= " #expected, __FILE__, __LINE__)

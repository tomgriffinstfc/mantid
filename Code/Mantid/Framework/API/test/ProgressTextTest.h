#ifndef MANTID_API_PROGRESSTEXTTEST_H_
#define MANTID_API_PROGRESSTEXTTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidKernel/Timer.h"
#include "MantidKernel/System.h"
#include <iostream>
#include <iomanip>
#include <cstdlib>

#include "MantidAPI/ProgressText.h"

using namespace Mantid::API;

class ProgressTextTest : public CxxTest::TestSuite
{
public:

  void test_setNumSteps()
  {
    ProgressText p(0.5, 1.0, 10);
    TS_ASSERT_THROWS_NOTHING(p.setNumSteps(100));
  }


  /// Disabled because it has text output
  void xtest_with_stdout()
  {
    ProgressText p(0.5, 1.0, 10);
    // 4 outputs
    p.report();
    p.report("I have an optional message");
    p.report();
    p.report();

    p.setNumSteps(100);
    // These should output only 2 lines. The % will go backwards though
    p.report();
    p.report();
    p.report();
    p.report();

    p.setNumSteps(5);
    p.report();
  }


//  /// Disabled because it has text output
//  void test_on_one_line()
//  {
//    ProgressText p(0.0, 1.0, 100, false);
//    for (int i=0; i<100; i++)
//    {
//      std::string msg = "";
//      for (int i = 0; i < std::rand() %10; i++)
//        msg += "bla";
//      p.report(msg);
//      usleep(10000);
//    }
//  }




};


#endif /* MANTID_API_PROGRESSTEXTTEST_H_ */


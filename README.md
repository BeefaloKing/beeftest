# BeefTest
A simple C++ unit testing framework written to waste my time on a Sunday afternoon. You should probably use something else as I guarantee this is worse. Modeled after [Catch](https://github.com/catchorg/Catch2).

## Writing Tests
Create a new source file and add these two lines at the top.
```C++
#define ENABLE_BEEF_MAIN
#include "beeftest.hh"
```
`#define ENABLE_BEEF_MAIN` provides a main to run your unit tests, so only add it to one file.  
Following that, include any production code you plan on testing and use the `BEEF_TEST(testname)` macro to write your tests. Replace `testname` with a unique name for every test.
```C++
#define ENABLE_BEEF_MAIN
#include "beeftest.hh"

#include "myProductionCode.hh"

BEEF_TEST(foobar)
{
	bool passed = true;
	/*
	 * Implement test here
	 */
	return passed;
}
```
`BEEF_TEST` expects you to define a function which returns a boolean. A return value of `true` indicates that the test passed.

## Running Tests
Your compiled test code will by default run all tests. After running it will display a count of tests executed and tests failed.
```
$ ./test.exe
1 tests executed!
0 tests failed!
```
All failing tests, if any, will be displayed.
```
$ ./test.exe
[foobar] (test/test_main.cpp:4) FAILED
1 tests executed!
1 tests failed!
```
A full log of all tests run (and whether they passed or failed) will be written to _testlog.txt_

You may pass a list of test names as command line arguments to only run specific tests, or alternatively use the `-f` option to pass a filename and run all tests in that file.  
`./test.exe foo bar foobar` executes tests _foo_, _bar_, and _foobar_.  
`./test.exe foo bar foobar -f test_feature.cpp` additionally executes all tests defined in _test_feature.cpp_.

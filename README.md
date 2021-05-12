# BeefTest
A simple C++ unit testing framework written to waste my time on a Sunday afternoon. You should probably use something else as I guarantee this is worse. Modeled after [Catch2](https://github.com/catchorg/Catch2).

## Writing Tests
Create a new source file and add these two lines at the top.
```C++
#define ENABLE_BEEF_TEST_MAIN
#include "beeftest.hh"
```
`#define ENABLE_BEEF_TEST_MAIN` provides a main to run your unit tests, so only add it to one file.  
Following that, include any production code you plan on testing and use the `beef_test("name")` macro to write your tests. Test names do not have to be unique, but they will be harder to distinguish if not.
```C++
#define ENABLE_BEEF_TEST_MAIN
#include "beeftest.hh"

#include "myproductioncode.hh"

beef_test("foobar")
{
	 /* Any setup if necessary.
	 */
	beef_cond(/* Expression. */);
}
```
`beef_test("name")` expects you to define a function which includes one or more `beef_cond(expression)` statements. For each included condition, the expression is saved, evaluated once, and it's result recorded. If one or more expressions evaluate to false, then the test fails.

If you would rather a test immediately fail and return when a given expression evaluates to false, use the `beef_assert(expression)` macro instead.
```C++
beef_test("fizz")
{
	/* Setup.
	 */
	beef_assert(myPtr != nullptr);
	beef_cond(myPtr->action());
}
```

## Running Tests
The included main will by default run all tests and print any failing tests (and their failing conditions) to stdout. A count of failed tests will also be displayed and set as the program's exit code.
```console
$ ./test.exe
test.cpp
:9 [FAIL] "foobar"
:12    [FAIL] (9 % -7 == -5)

Tests run: 2
Tests failed: 1
```
The verbosity of the output can be changed with the `-v` option between levels `-v0` and `-v3` (default is `-v1`).

Rather than running all tests, individual test names may be passed as arguments or, alternatively, the `-f` option may be used to pass in a filename.  
`./test.exe foo bar foobar` executes tests "foo", "bar", and "foobar".  
`./test.exe foo bar foobar -f test_feature.cpp` additionally executes all tests defined in _test_feature.cpp_.

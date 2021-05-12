// BeefTest 2.0
// Written (poorly) by Beefalo
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace beeftest {
class Test;
struct Logger;
struct Result;
typedef void(*TestFunc)(std::vector<Result> &);
typedef bool(*CmpFunc)(const Test*, const Test*);

class Test
{
private:
	// Singleton getter functions.
	inline static std::multimap<std::string, Test*> &byName();
	inline static std::multimap<std::string, Test*> &byFile();
	inline static std::set<Test*, CmpFunc> &tests(); // Selected tests.

	inline static bool cmp(const Test* left, const Test* right);
	inline static bool select(const std::string &key, const std::multimap<std::string, Test*> &map);
public:
	const std::string name;
	const std::string file;
	const size_t line;
	const TestFunc test;

	inline Test();
	inline Test(const std::string &name, const std::string &file, size_t line, TestFunc test);

	inline static size_t &maxLine(size_t line = 0); // Used for pretty printing.

	inline static bool runName(const std::string &name); // Select tests by name.
	inline static bool runFile(const std::string &file); // Select tests by file.
	inline static size_t run(const Logger &logger); // Runs selected tests (defaults to all).
};

struct Logger
{
	enum class Level
	{
		failTests = 0,
		failAsserts,
		allTests,
		allAsserts
	};
	Level verbosity = Level::failAsserts;

	inline void log(const Test* test, bool testPass, const std::vector<Result> &results) const;
	inline void log(const std::string &msg) const;
};

// Holds result of a single assertion.
struct Result
{
	std::string expression;
	size_t line;
	bool pass;
};

// Random helper functions.
inline size_t digits(size_t n)
{
	size_t digits = 1;
	while ((n /= 10) > 0)
	{
		++digits;
	}
	return digits;
}

// class Test
Test::Test(const std::string &name, const std::string &file, size_t line, TestFunc test) :
	name{name},
	file{file},
	line{line},
	test{test}
{
	byName().emplace(name, this);
	byFile().emplace(file, this);
	maxLine(line);
}

std::multimap<std::string, Test*> &Test::byName()
{
	static std::multimap<std::string, Test*> byName_;
	return byName_;
}
std::multimap<std::string, Test*> &Test::byFile()
{
	static std::multimap<std::string, Test*> byFile_;
	return byFile_;
}
std::set<Test*, CmpFunc> &Test::tests()
{
	static std::set<Test*, CmpFunc> tests_{cmp};
	return tests_;
}
size_t &Test::maxLine(size_t line)
{
	static size_t maxLine_ = 0;
	return maxLine_ = (line > maxLine_ ? line : maxLine_);
}

bool Test::cmp(const Test* left, const Test* right)
{
	return left->file != right->file ? left->file < right->file
		: left->line != right->line ? left->line < right->line
		: left->test < right->test;
}

bool Test::select(const std::string &key, const std::multimap<std::string, Test*> &map)
{
	auto [first, last] = map.equal_range(key);
	for (auto it = first; it != last; ++it)
	{
		tests().insert(it->second);
	}
	return first != last; // Returns true if at least one test was found in selection.
}

bool Test::runName(const std::string &name)
{
	return select(name, byName());
}

bool Test::runFile(const std::string &file)
{
	return select(file, byFile());
}

size_t Test::run(const Logger &logger)
{
	if (tests().empty()) // Select all tests by name if none selected.
	{
		for (const auto &[key, value] : byName())
		{
			tests().insert(value);
		}
	}

	size_t testsFailed = 0;
	for (Test* t : tests())
	{
		std::vector<Result> results;
		t->test(results);

		bool testPass = true;
		for (const auto &r : results)
		{
			testPass &= r.pass;
		}
		testsFailed += !testPass;

		logger.log(t, testPass, results);
	}

	logger.log("\n");
	logger.log("Tests run: " + std::to_string(tests().size()) + "\n");
	logger.log("Tests failed: " + std::to_string(testsFailed) + "\n");
	return testsFailed;
}

// struct Logger
void Logger::log(const Test* test, bool testPass, const std::vector<Result> &results) const
{
	static std::string currentFile;

	if (verbosity >= (testPass ? Level::allTests : Level::failTests))
	{
		if (test->file != currentFile)
		{
			std::cout << (currentFile != "" ? "\n" : "") // Skip newline before first filename.
				<< (currentFile = test->file) << "\n";
		}

		std::cout << ":" << std::left << std::setw(digits(Test::maxLine()) + 1) << test->line
			<< (testPass ? "[PASS]" : "[FAIL]") << " \"" << test->name << "\"\n";

		for (const auto &r : results)
		{
			if (verbosity >= (r.pass ? Level::allAsserts : Level::failAsserts))
			{
				std::cout << ":" << std::left << std::setw(digits(Test::maxLine()) + 5) << r.line
					<< (r.pass ? "[PASS]" : "[FAIL]") << " " << r.expression << "\n";
			}
		}

		std::cout.flush(); // Flush buffer after each test.
	}
}

void Logger::log(const std::string &msg) const
{
	std::cout << msg;
}

} // namespace beeftest

#define BEEF_UNIQ(PREFIX, LINE) PREFIX ## LINE
#define BEEF_UNIQ_FUNC(LINE) BEEF_UNIQ(BEEF_FUNC_, LINE)
#define BEEF_UNIQ_OBJ(LINE) BEEF_UNIQ(BEEF_OBJ_, LINE)

#define beef_test(NAME) \
void BEEF_UNIQ_FUNC(__LINE__)(std::vector<beeftest::Result> &); \
beeftest::Test BEEF_UNIQ_OBJ(__LINE__){NAME, __FILE__, __LINE__, BEEF_UNIQ_FUNC(__LINE__)}; \
void BEEF_UNIQ_FUNC(__LINE__)(std::vector<beeftest::Result> &results)

#define beef_cond(EXPR) \
results.push_back(beeftest::Result{"beef_cond(" #EXPR ")", __LINE__, (EXPR)});

#define beef_assert(EXPR) \
results.push_back(beeftest::Result{"beef_assert(" #EXPR ")", __LINE__, (EXPR)}); \
if (results.back().pass == false) return;

#ifdef ENABLE_BEEF_TEST_MAIN
int main(int argc, char* argv[])
{
	beeftest::Logger logger;

	std::string pFlags = "fv"; // Flags that take parameters.
	for (size_t i = 1; i < argc; ++i)
	{
		char* arg = argv[i];
		if (arg[0] != '-')
		{
			if (!beeftest::Test::runName(arg))
			{
				std::cout << "No tests found for name \"" << arg << "\"\n";
				return -1;
			}
			continue;
		}

		char flag = arg[1];
		char* param = nullptr;
		if (pFlags.find(flag) != std::string::npos) // Check if flag requires parameter.
		{
			param = (arg[2] != '\0') ? arg + 2 :
				(++i < argc) ? argv[i] : nullptr;
			if (param == nullptr)
			{
				std::cout << "Missing parameter for option -" << flag << "\n";
				return -1;
			}
		}

		switch (flag)
		{
		case 'f':
			if (!beeftest::Test::runFile(param))
			{
				std::cout << "No tests found for file \"" << param << "\"\n";
				return -1;
			}
			continue;
		case 'v':
			try
			{
				logger.verbosity = beeftest::Logger::Level{std::stoi(param)};
			}
			catch (const std::invalid_argument &e)
			{
				std::cout << "Verbosity level must be an integer.\n";
				return -1;
			}
			continue;
		default:
			std::cout << "Unknown option -" << flag << "\n";
			return -1;
		}
	}

	size_t testsFailed = beeftest::Test::run(logger);
	return testsFailed < 256 ? testsFailed : 255;
}
#endif // ENABLE_BEEF_TEST_MAIN

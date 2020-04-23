// BeefTest v1.2
// Written by Beefalo
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace BEEF
{
	class TestCase;
}

class BEEF::TestCase
{
public:
	TestCase(std::string name, void (*test)(std::vector<std::string>&), std::string file, int line) :
		name(name), file(file), line(line), test(test)
	{
		getTests().emplace(name, this);
		auto it = getFiles().emplace(file, NULL).first;
		it->second.push_back(this);
	}

	void run(std::vector<std::string>& failedAsserts) const
	{
		test(failedAsserts);
	}

	void print(std::ostream& out)
	{
		out << "[" << name << "] (" << file << ":" << line << ")";
	}

	// Executes tests with names listed by names or defined in files listed by files
	// Executes all tests by default if both names and files are empty
	static int runTests(std::set<std::string> testNames, std::set<std::string> fileNames)
	{
		// Add all test names in each file to testNames
		for (const auto& file : fileNames)
		{
			auto it = getFiles().find(file);
			if (it == getFiles().end())
			{
				std::cout << "File \"" << file << "\" not found or contains no tests!\n";
				return -1;
			}
			for (const auto& test : it->second)
			{
				testNames.emplace(test->name);
			}
		}

		std::vector<TestCase*> toRun;
		// Add all tests whose name exists in testNames to toRun
		// If no test names supplied, default to all tests
		if (testNames.empty())
		{
			for (const auto& test : getTests())
			{
				toRun.push_back(test.second);
			}
		}
		else
		{
			for (const auto& name : testNames)
			{
				auto it = getTests().find(name);
				if (it == getTests().end())
				{
					std::cout << "Test \"" << name << "\" not found!\n";
					return -1;
				}
				toRun.push_back(it->second);
			}
		}
		return runTestList(toRun);
	}
private:
	std::string name;
	std::string file;
	int line;
	void (*test)(std::vector<std::string>&); // test pointer;

	static std::map<std::string, TestCase*>& getTests()
	{
		static std::map<std::string, TestCase*> tests;
		return tests;
	}

	static std::map<std::string, std::vector<TestCase*>>& getFiles()
	{
		static std::map<std::string, std::vector<TestCase*>> files;
		return files;
	}

	static int runTestList(std::vector<TestCase*> list)
	{
		std::ofstream log;
		log.open("testlog.txt", std::ofstream::trunc);

		int failed = 0;
		std::vector<std::string> failedAssertions;
		for (const auto& test : list)
		{
			test->print(log);
			log << std::flush;

			failedAssertions.clear();
			test->run(failedAssertions);

			if (failedAssertions.empty())
			{
				log << " PASSED\n";
			}
			else
			{
				failed++;
				test->print(std::cout);
				std::cout << " FAILED\n";
				log << " FAILED\n";

				for (const auto& expression : failedAssertions)
				{
					std::cout << "  ASSERT(" << expression << "); FAILED\n";
					log << "  ASSERT(" << expression << "); FAILED\n";
				}
			}
		}
		std::cout << list.size() << " tests executed!\n" << failed << " tests failed!\n";

		log.close();
		return failed;
	}
};

#define BEEF_TEST(TESTNAME)\
	void BEEF_FUN_##TESTNAME(std::vector<std::string>&);\
	BEEF::TestCase BEEF_OBJ_##TESTNAME {#TESTNAME, BEEF_FUN_##TESTNAME, __FILE__, __LINE__};\
	void BEEF_FUN_##TESTNAME(std::vector<std::string>& BEEF_FAILED_EXPRESSIONS)

#define ASSERT(EXPRESSION)\
	if (!(EXPRESSION)) BEEF_FAILED_EXPRESSIONS.push_back(#EXPRESSION);

#ifdef ENABLE_BEEF_MAIN
int main(int argc, char* argv[])
{
	std::set<std::string> names;
	std::set<std::string> files;

	// argv[0] is the path of the executable and skipped
	for (int i = 1; i < argc; i++)
	{
		char* arg = argv[i];
		if (arg[0] == '-' && arg[1] != '\0') // Flag character
		{
			char flag = arg[1];
			char* flagArg = nullptr;
			if (arg[2] != '\0')
			{
				flagArg = arg + 2;
			}
			else if (i + 1 < argc)
			{
				flagArg = argv[++i];
			}

			switch (flag)
			{
				case 'f':
					if (flagArg == nullptr)
					{
						std::cout << "Option " << arg << " requires parameter\n";
					}
					files.emplace(flagArg);
					continue;
				default:
					std::cout << "Unknown option " << arg << "\n";
					return -1;
			}
		}

		// If argument was not a flag add it to names
		names.emplace(arg);
	}

	return BEEF::TestCase::runTests(names, files);
}
#endif //ENABLE_BEEF_MAIN

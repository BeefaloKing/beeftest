// BeefTest v1.1
// Written by Beefalo
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

class TestCase;
typedef std::map<std::string, TestCase*> testMap;

typedef std::vector<TestCase*> testVector;
typedef std::map<std::string, testVector> fileMap;

typedef std::set<std::string> nameSet;

class TestCase
{
public:
	TestCase(std::string name, bool (*test)(), std::string file, int line) :
		name(name), file(file), line(line), test(test)
	{
		getTests().emplace(name, this);
		auto it = getFiles().emplace(file, NULL).first;
		it->second.push_back(this);
	}

	bool run() const
	{
		return test();
	}

	void print(std::ostream& out)
	{
		out << "[" << name << "] (" << file << ":" << line << ")";
	}

	// Executes tests with names listed by names or defined in files listed by files
	// Executes all tests by default if both names and files are empty
	static int runTests(nameSet testNames, nameSet fileNames)
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

		testVector toRun;
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
	bool (*test)();// test;

	static testMap& getTests()
	{
		static testMap tests;
		return tests;
	}

	static fileMap& getFiles()
	{
		static fileMap files;
		return files;
	}

	static int runTestList(testVector list)
	{
		std::ofstream log;
		log.open("testlog.txt", std::ofstream::trunc);

		int failed = 0;
		for (const auto& test : list)
		{
			test->print(log);
			log << std::flush;
			if (test->run())
			{
				log << " PASSED\n";
			}
			else
			{
				failed++;
				test->print(std::cout);
				std::cout << " FAILED\n";
				log << " FAILED\n";
			}
		}
		std::cout << list.size() << " tests executed!\n" << failed << " tests failed!\n";

		log.close();
		return failed;
	}
};

#define BEEF_TEST(TESTNAME)\
	bool BEEF_FUN_##TESTNAME();\
	TestCase BEEF_OBJ_##TESTNAME {#TESTNAME, BEEF_FUN_##TESTNAME, __FILE__, __LINE__};\
	bool BEEF_FUN_##TESTNAME()

#ifdef ENABLE_BEEF_MAIN
int main(int argc, char* argv[])
{
	nameSet names;
	nameSet files;

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

	return TestCase::runTests(names, files);
}
#endif //ENABLE_BEEF_MAIN

#include <iostream>
#include <fstream>
#include "Map.hpp"

using namespace std;

void writeanswer(const string& answerPath, const vector<string>& answer)
{
	ofstream out(answerPath);
	for (auto&line : answer)
		out << line << endl;
}


int main(int argc, char *argv[])
{
	if(argc < 5)
	{
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		exit(1);
	}

	string carPath(argv[1]);
	string roadPath(argv[2]);
	string crossPath(argv[3]);
	string answerPath(argv[4]);

	Map map;
	map.load(carPath, roadPath, crossPath);
	int minruntime = INT32_MAX;
	vector<string> minanswer;
	for (int i = 0; i < 3; ++i)
	{
		int runtime = map.run();
		cout << "调度时间:" << runtime << endl;
		if (map.run() < minruntime)
			minanswer = map.answer();
	}
	writeanswer(answerPath, minanswer);
	return 0;
}
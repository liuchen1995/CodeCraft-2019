#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "Map.hpp"


using namespace std;


// 将答案写到文件里
void writeanswer(const string& answerPath, const map<int, vector<int>>& answer)
{
	ofstream out(answerPath);
	for (auto&line : answer)
	{
		const vector<int>& theline = line.second;
		out << "(" << theline[0] << "," << theline[1];
		for (size_t i = 2; i < theline.size(); ++i)
			out << "," << theline[i];
		out << ")" << endl;
	}
	out.close();
}

// 读取答案文件
const map<int, vector<int>> readanswer(const string& answerPath)
{
	map<int, vector<int>> res;
	string line;
	ifstream in(answerPath);
	while (getline(in, line))
	{
        for (auto&i: line)
		{
            if (i == '(' || i == ')' || i == ',')
                i = ' ';
		}
		istringstream iss(line);
		int carid, starttime;
		iss >> carid >> starttime;
		vector<int> r;
		r.push_back(carid);
		r.push_back(starttime);
		int roadid;
		while (iss >> roadid)
			r.push_back(roadid);
		res.insert(pair<int, vector<int>>(carid, r));
	}
	in.close();
	return res;
}

//输出运行结果
void outputresult(const map<string, int>& res)
{
	if (res.size() == 0)
		cout << "不要怕死锁" << endl;

	else
	{
		cout << "优先车辆运行结果: {调度时间: " << res.at("specialscheduletime") << ", 车辆总调度时间: " << res.at("specialallscheduletime") << "}" << endl;
		cout << "所有车辆运行结果: {调度时间: " << res.at("originscheduletime") << ", 车辆总调度时间: " << res.at("originallscheduletime") << "}" << endl;
		cout << "最终调度时间: " << res.at("endscheduletime") << ", 最终车辆总调度时间: " << res.at("endallscheduletime") << endl;
	}
}

int main(int argc, char *argv[])
{
	if(argc < 6)
	{
		std::cout << "please input args: carPath, roadPath, crossPath, answerPath" << std::endl;
		exit(1);
	}

	string carPath(argv[1]);
	string roadPath(argv[2]);
	string crossPath(argv[3]);
	string presetAnswerPath(argv[4]);
	string answerPath(argv[5]);

	Map m;
	m.load(carPath, roadPath, crossPath, presetAnswerPath);
	map<string, int> res;

	//参数1：第一阶段路面车辆数限流
	//参数2：第二阶段路面车辆限流
	//参数3：第一阶段路面车辆每时刻放置数
	//参数4：第二阶段路面车辆每时刻放置数
	//参数5：道路系数，越小车子路径越接近最短距离，但是容易死锁，越大，车子会转圈

	if (m.getcrosssnum() == 141)
		res = m.simulation(2000, 3800, 40, 180, 0.98);
	else if (m.getcrosssnum() == 165)
		res = m.simulation(3000, 3500, 40, 100, 0.95);
	else
		res = m.simulation();
		
	cout << "本地规划运行结果:" << endl;
	outputresult(res);
	if (res.size() != 0)
		writeanswer(answerPath, m.answer());
	

	// auto readedanswer = map.judger(readanswer(answerPath));
	// cout << "本地判题器运行结果:" << endl;
	// outputresult(readedanswer);

	return 0;
}
#ifndef MAP_
#define MAP_


#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <math.h>
#include "Car.hpp"
#include "Road.hpp"
#include "Cross.hpp"


using namespace std;

class Map
{
    public:
        Map() {}
        ~Map();
        void load(const string& carpath, const string& roadpath, const string& crosspath);
        void output(int clock) const;
        int run(float roadcoefficient = 1.5);
        vector<string> answer() const;
    private:
        void letcaronroad(Car* const thecar, Cross* const thecross, int clock);
        bool isallcaronover() const;
        bool isallcarsatdest() const;
        int getstepofcarcango(Car* const thecar, Road* const theroad) const;
        void updateminpathtable(float roadcoefficient);
        void updatecarpath(Car* const thecar);
        bool iscarconficted(Cross*const thecross, Road*const theroad, Car* const thecar) const;
        void driveonechannelcars(Road* const theroad, const int channel);
        Car* getfirstprioritycar(Road*const theroad) const;
        bool drivecartonextroad(Cross* const thecross, Road*const theroad, Car* const thecar);
        bool istheroadhasthedirectioncar(Cross*const thecross, Road*const theroad, Cross::DIRECTION direction) const;
        void getnextroadchannelandstep(Road* const nextroad, const int nextability, int& nextchannel, int& nextstep) const;
        int carpathcongestionrate(Car* const thecar, const int calcnumbersofpath) const;
        void setcarpath(Car* const thecar);
        int placecarsabilityofthecross(Cross*const thecross) const;
        int _mapcapacity = 0; //地图承载车辆的能力
        int _placecarsabilityofallcrosss = 0;
        int _carnumberinmap = 0; //统计路段上的车辆数
        int _carnumberofrelease = 0; //统计当前放车的数量
        map<pair<int, int>, vector<int>> _minpathtable; //最短路径查找表
        vector<vector<int>> _mindisttable; //最短路径距离查找表
        map<pair<int, int>, Road*> _roads; //存放路段
        map<int, Cross*> _crosss; //存放路口
        map<int, Car*> _cars; //存放车辆
        vector<Cross*> _sortedcrosss;
        vector<Cross*> _idrisecrosss;
};

// 读取car.txt, cross.txt, road.txt文件，并构造地图和车辆，计算最短路径表
void Map::load(const string& carpath, const string& roadpath, const string& crosspath)
{
    
    string line;
    //读取路口，并构造路口
    ifstream in(crosspath, ifstream::in);
    while (getline(in, line))
    {
        if (line[0] == '#')
            continue;
        for (auto&i: line)
            if (i == '(' || i == ')' || i == ',')
                i = ' ';
        istringstream iss(line);
        int id, roadid0, roadid1, roadid2, roadid3; 
        iss >> id >> roadid0 >> roadid1 >> roadid2 >> roadid3;
        _crosss.insert(pair<int, Cross*>(id, new Cross(id, roadid0, roadid1, roadid2, roadid3)));
        _sortedcrosss.push_back(_crosss.at(id));
        _idrisecrosss.push_back(_crosss.at(id));
    }
    in.close();

    sort(_idrisecrosss.begin(), _idrisecrosss.end(), [](Cross*const a, Cross*const b) -> bool { return a->id() < b->id(); });

    //读取路段，并构造路段
    in.open(roadpath, ifstream::in);
    while (getline(in, line))
    {
        if (line[0] == '#')
            continue;
        for (auto&i: line)
            if (i == '(' || i == ')' || i == ',')
                i = ' ';
        istringstream iss(line);
        int id, length, speed, channel, from, to;
        bool isduplex;
        iss >> id >> length >> speed >> channel >> from >> to >> isduplex;
        if (isduplex)
        {
            _roads.insert(pair<pair<int, int>, Road*>(pair<int, int>(from, to), new Road(id,length,speed,channel,from,to)));
            _roads.insert(pair<pair<int, int>, Road*>(pair<int, int>(to, from), new Road(id,length,speed,channel,to,from)));
            _mapcapacity += length * channel * 2;
            
        } else
        {
            _roads.insert(pair<pair<int, int>, Road*>(pair<int, int>(from, to), new Road(id,length,speed,channel,from,to)));
            _mapcapacity += length * channel;
        }
    }
    in.close();

    //读取汽车，并构造汽车
    in.open(carpath, ifstream::in);
    while (getline(in, line))
    {
        if (line[0] == '#')
            continue;
        for (auto&i: line)
            if (i == '(' || i == ')' || i == ',')
                i = ' ';
        istringstream iss(line);
        int id, from, to, speed, plantime;
        iss >> id >> from >> to >> speed >> plantime;
        _cars.insert(pair<int, Car*>(id, new Car(id, from, to, speed, plantime)));
    }
    in.close();

    //设置路口与道路的连接
    for(auto& cross: _crosss)
    {
        Cross* thecross = cross.second;
        const vector<int>& roadids = thecross->roadids();
        map<int, Road*> intemp;
        map<int, Road*> outtemp;
        for (auto& roadid: roadids)
        {
            for (auto& i: _idrisecrosss)
                if (_roads.find(pair<int, int>(i->id(), thecross->id())) != _roads.end() && _roads.at(pair<int, int>(i->id(), thecross->id()))->id() == roadid)
                {
                    intemp.insert(pair<int, Road*>(roadid, _roads.at(pair<int, int>(i->id(), thecross->id()))));
                    break;
                }
            for (auto& i: _idrisecrosss)
                if (_roads.find(pair<int, int>(thecross->id(), i->id())) != _roads.end() && _roads.at(pair<int, int>(thecross->id(), i->id()))->id() == roadid)
                {
                    outtemp.insert(pair<int, Road*>(roadid, _roads.at(pair<int, int>(thecross->id(), i->id()))));
                    break;
                }
        }
        thecross->setincrossroads(intemp);
        thecross->setoutcrossroads(outtemp);
    }

    //计算总路口放置车辆能力
    for (auto& thecross: _crosss)
        _placecarsabilityofallcrosss += placecarsabilityofthecross(thecross.second);

}

Map::~Map()
{
    for (auto& i : _roads)
        delete i.second;
    for (auto &i: _crosss)
        delete i.second;
    for (auto &i: _cars)
        delete i.second;
}

// 输出答案answer.txt
vector<string> Map::answer() const
{
    vector<string> res;

    for (auto& car: _cars)
    {
        string line;
        Car * thecar = car.second;
        line = "(" + to_string(thecar->id()) + "," + to_string(thecar->starttime());
        for (auto& theroad: thecar->roads())
            line += "," + to_string(theroad->id());
        line += ")";
        res.push_back(line);
    }
    return res;
}

// 更新最短路径表
void Map::updateminpathtable(float roadcoefficient)
{

    //floyd算法计算最短路径表
    _mindisttable =  vector<vector<int>>(_idrisecrosss.size(), vector<int>(_idrisecrosss.size(), INT32_MAX));
    vector<vector<int>> path(_idrisecrosss.size(), vector<int>(_idrisecrosss.size()));
    for (size_t i = 0; i < _idrisecrosss.size(); ++i)
    {
        for (size_t j = 0; j < _idrisecrosss.size(); ++j)
        {
            path[i][j] = j;
            if (_roads.find(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())) == _roads.end())
                continue;
            _mindisttable[i][j] = _roads.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id()))->w(roadcoefficient);
        }
    }

    for (size_t k = 0; k < _idrisecrosss.size(); ++k)
    {
        for (size_t i = 0; i < _idrisecrosss.size(); ++i)
        {
            for (size_t j = 0; j < _idrisecrosss.size(); ++j)
            {
                int s = (_mindisttable[i][k] == INT32_MAX || _mindisttable[k][j] == INT32_MAX)? INT32_MAX: _mindisttable[i][k] + _mindisttable[k][j];
                if (s < _mindisttable[i][j])
                {
                    _mindisttable[i][j] = s;
                    path[i][j] = path[i][k];
                }
            }
        }
    }
    //更新最短路径表
    _minpathtable = map<pair<int, int>, vector<int>>();
    for (size_t i = 0; i < _idrisecrosss.size(); ++i)
    {
        for (size_t j = 0; j < _idrisecrosss.size(); ++j)
        {
            if (i == j)
                continue;
            if (_minpathtable.find(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())) == _minpathtable.end())
                _minpathtable.insert(pair<pair<int, int>, vector<int>>(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id()), vector<int>()));
            _minpathtable.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())).push_back(_idrisecrosss[i]->id());
            size_t temp = path[i][j];
            while (temp != j)
            {
                _minpathtable.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())).push_back(_idrisecrosss[temp]->id());
                temp = path[temp][j];
            }
            _minpathtable.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())).push_back(_idrisecrosss[j]->id());
        }
    }
}

// 判断车辆是否都终止运行
bool Map::isallcaronover() const
{
    for (auto&car: _cars)
    {
        if (car.second->state() == Car::STATE::ON_WAIT)
            return false;
    }
    return true;
}

// 计算车子在该道路最多可以走多少步
int Map::getstepofcarcango(Car* const thecar, Road* const theroad) const
{
    int speed = min(thecar->speed(), theroad->speed());
    int step = 0;
    for (int i = thecar->posid() + 1; i < min(thecar->posid() + speed + 1, theroad->length()); i++)
    {
        if (theroad->poshasacar(thecar->channelid(), i) == false)
            step++;
        else
            break;
    }
    return step;
}

// 计算车子去下一道路具体位置
void Map::getnextroadchannelandstep(Road* const nextroad, const int nextability, int& nextchannel, int& nextstep) const
{
    nextchannel = 0;
    nextstep = 0;
    for (; nextchannel < nextroad->channel(); ++nextchannel)
    {
        for (int n = 0; n < nextability; ++n)
        {
            if (nextroad->poshasacar(nextchannel, n) == false)
                ++nextstep;
            else
                break;
        }
        if (nextstep > 0)
            break;
    }
}

// 判断这一条道路有没有这一方向的车子过路口
bool Map::istheroadhasthedirectioncar(Cross* const thecross, Road* const theroad, Cross::DIRECTION direction) const
{
    Car* thecar = getfirstprioritycar(theroad);
    if (thecar == nullptr)
        return false; //没有等待过路口的车辆
    int step = getstepofcarcango(thecar, theroad);
    int speed = min(thecar->speed(), theroad->speed());
    Road* thenextroad = thecar->getnextroad();
    if (thenextroad == nullptr)
    {
        if (thecar->posid() + step + 1 == theroad->length() && speed > step && direction == Cross::DIRECTION::STRAIGHT)
            return true;
        else
            return false;
    }
    int nextability = min(thenextroad->speed(), thecar->speed()) - step;
    if (thecar->posid() + step + 1 < theroad->length() || speed == step || nextability < 1)
        return false; //没有以上条件就不会过路口
    if (thecross->judgedirection(theroad->id(), thenextroad->id()) == direction) //是这一方向的
    {
        int nextchannel = 0;
        int nextstep = 0;
        getnextroadchannelandstep(thenextroad, nextability, nextchannel, nextstep);
        if (nextstep > 0)
            return true; //有位置
        else if (nextstep == 0)
        {
            if (thenextroad->getcarfrompos(thenextroad->channel() - 1, 0)->state() == Car::STATE::ON_OVER)
                return false; //没有有位置
            else if (thenextroad->getcarfrompos(thenextroad->channel() - 1, 0)->state() == Car::STATE::ON_WAIT)
                return true; //说不定有位置是吧
        }
    }
    return false;
}


//移动theroad的channel通道的所有车，第一步和第二步都会调用
void Map::driveonechannelcars(Road* const theroad, const int channel)
{
    for (int pos = theroad->length() - 1; pos >= 0; --pos) //从头到尾遍历
    {
        if (theroad->poshasacar(channel, pos) == false || theroad->getcarfrompos(channel, pos)->state() == Car::STATE::ON_OVER)
            continue;
        //走到这里这辆车是第一步ON_READY或第二步ON_WAIT
        Car* thecar = theroad->getcarfrompos(channel, pos);
        int speed = min(thecar->speed(), theroad->speed());
        int step = getstepofcarcango(thecar, theroad);
        if ((pos + step + 1 < theroad->length() && speed > step && theroad->getcarfrompos(channel, pos + step + 1)->state() == Car::STATE::ON_WAIT) || 
            (pos + step + 1 == theroad->length() && speed > step))  //当前车道有阻挡且前面车子为wait或者能过路口
        {
            thecar->setstate(Car::STATE::ON_WAIT);
            continue;
        }
        else if ((pos + step + 1 < theroad->length() && speed > step && theroad->getcarfrompos(channel, pos + step + 1)->state() == Car::STATE::ON_OVER) || 
            (pos + step + 1 <= theroad->length() && speed == step)) //当前车道有阻挡且前车为over或者当前道路无阻挡行驶最大距离
        {
            thecar->setstate(Car::STATE::ON_OVER);
            thecar->setposid(pos + step);
            theroad->setposnocar(channel, pos);
            theroad->setposacar(channel, pos + step, thecar);
            continue;
        }
    }
}

//判断第一优先级的车辆是否矛盾吗
bool Map::iscarconficted(Cross* const thecross, Road* const theroad, Car* const thecar) const
{
    Road* thenextroad = thecar->getnextroad();
    if (thenextroad == nullptr)
        return false; //到达终点的车相当于直走不矛盾
    int step = getstepofcarcango(thecar, theroad);
    int speed = min(thecar->speed(), theroad->speed());
    int nextability = min(thenextroad->speed(), thecar->speed()) - step;
    if (thecar->posid() + step + 1 < theroad->length() || speed == step || nextability < 1)
        return false; //没有以上条件就不会过路口，不矛盾
    int nextchannel = 0, nextstep = 0;
    getnextroadchannelandstep(thenextroad, nextability, nextchannel, nextstep);
    if ((0 < nextstep && nextstep < nextability && thenextroad->getcarfrompos(nextchannel, nextstep)->state() == Car::STATE::ON_WAIT) || 
        (nextstep == 0 && thenextroad->getcarfrompos(thenextroad->channel() - 1, 0)->state() == Car::STATE::ON_WAIT) || 
        (nextstep == nextability && nextchannel > 0 && thenextroad->getcarfrompos(nextchannel - 1, 0)->state() == Car::STATE::ON_WAIT))
        return true; //转弯的那条路有车等待此车也等待
    if (thecross->judgedirection(theroad->id(), thenextroad->id()) == Cross::DIRECTION::STRAIGHT)
        return false;
    Road* theleftroad = nullptr, * thefrontroad = nullptr, * therightroad = nullptr;
    thecross->getotherdirectionincrossroads(theroad->id(), theleftroad, thefrontroad, therightroad);
    if (thecross->judgedirection(theroad->id(), thenextroad->id()) == Cross::DIRECTION::LEFT)
    {
        if (therightroad != nullptr && istheroadhasthedirectioncar(thecross, therightroad, Cross::DIRECTION::STRAIGHT) == true)
            return true;
        else
            return false;
    }
    else if (thecross->judgedirection(theroad->id(), thenextroad->id()) == Cross::DIRECTION::RIGHT)
    {
        if ((theleftroad != nullptr && istheroadhasthedirectioncar(thecross, theleftroad, Cross::DIRECTION::STRAIGHT) == true) || 
            (thefrontroad != nullptr && istheroadhasthedirectioncar(thecross, thefrontroad, Cross::DIRECTION::LEFT) == true))
            return true;
        else
            return false;
    }
    throw;
}

//将车辆移动到下一路口，如果车子过了路口返回true, 否则返回false
bool Map::drivecartonextroad(Cross* const thecross, Road* const theroad, Car* const thecar)
{
    int oldposid = thecar->posid();
    int oldchannelid = thecar->channelid();
    int step = getstepofcarcango(thecar, theroad);
    int speed = min(thecar->speed(), theroad->speed());
    if ((oldposid + step + 1 < theroad->length() && speed > step) || (oldposid + step + 1 <= theroad->length() && speed == step))
    {
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(oldposid + step);
        theroad->setposnocar(oldchannelid, oldposid);
        theroad->setposacar(oldchannelid, oldposid + step, thecar);
        return false;
    }
    Road* thenextroad = thecar->getnextroad();
    if (thenextroad == nullptr)
    {
        if (oldposid + step + 1 == theroad->length() && speed > step)
        {
            thecar->setstate(Car::STATE::AT_DEST);
            thecar->setposid(0);
            thecar->setchannelid(0);
            thecar->setroad(nullptr);
            theroad->setposnocar(oldchannelid, oldposid);
            thecross->pushdestacar(thecar);
            --_carnumberinmap;
            return true;
        }
        else 
        {
            thecar->setstate(Car::STATE::ON_OVER);
            thecar->setposid(oldposid + step);
            theroad->setposnocar(oldchannelid, oldposid);
            theroad->setposacar(oldchannelid, oldposid + step, thecar);
            return false;
        }
    }
    int nextability = min(thenextroad->speed(), thecar->speed()) - step;
    if (nextability < 1)
    {
        //步数不够车子还不能转, 在这条道最前面终止
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(oldposid + step);
        theroad->setposnocar(oldchannelid, oldposid);
        theroad->setposacar(oldchannelid, oldposid + step, thecar);
        return false;
    }
    int nextchannel = 0;
    int nextstep = 0;
    getnextroadchannelandstep(thenextroad, nextability, nextchannel, nextstep);
    if ((0 < nextstep && nextstep < nextability && thenextroad->getcarfrompos(nextchannel, nextstep)->state() == Car::STATE::ON_OVER) ||
        (nextstep == nextability && nextchannel > 0 && thenextroad->getcarfrompos(nextchannel - 1, 0)->state() == Car::STATE::ON_OVER) ||
        (nextstep == nextability && nextchannel == 0))
    {
        //过路口了
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(nextstep - 1);
        thecar->setchannelid(nextchannel);
        theroad->setposnocar(oldchannelid, oldposid);
        thenextroad->setposacar(nextchannel, nextstep - 1, thecar);
        thecar->setroad(thenextroad);
        return true;
    }
    else if (nextstep == 0 && thenextroad->getcarfrompos(thenextroad->channel() - 1, 0)->state() == Car::STATE::ON_OVER)
    {
        //下一条路太堵，只能在道路的最前方终止
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(oldposid + step);
        theroad->setposnocar(oldchannelid, oldposid);
        theroad->setposacar(oldchannelid, oldposid + step, thecar);
        return false;
    }
    throw;
}

Car* Map::getfirstprioritycar(Road*const theroad) const
{
    for (int i = theroad->length() - 1; i >= 0; i--)
    {
        for (int j = 0; j < theroad->channel(); j++)
        {
            if (theroad->poshasacar(j, i) == false || theroad->getcarfrompos(j, i)->state() == Car::STATE::ON_OVER) //此位置没有车或者车子处于over状态，处于over状态的车子不参与优先级排序
                continue;
            return theroad->getcarfrompos(j, i);
        }
    }
    return nullptr;
}

// 让车上路
void Map::letcaronroad(Car* const thecar, Cross* const thecross, int clock)
{
    //开始让车子上路
    Road* nextroad = thecar->getnextroad();
    int nextability = min(thecar->speed(), nextroad->speed());
    int nextchannel = 0, nextstep = 0;
    getnextroadchannelandstep(nextroad, nextability, nextchannel, nextstep);
    if (nextstep == 0)
        return;
    nextroad->setposacar(nextchannel, nextstep - 1, thecar);
    thecar->setstarttime(clock);
    thecar->setchannelid(nextchannel);
    thecar->setposid(nextstep - 1);
    thecar->setstate(Car::STATE::ON_OVER);
    thecar->setroad(nextroad);
    thecross->popgarageacar(thecar);
    _carnumberinmap++;
    _carnumberofrelease++;
}

// 判断是不是所有车子到达终点
bool Map::isallcarsatdest() const
{
    for (auto& car: _cars)
    {
        if (car.second->state() != Car::STATE::AT_DEST)
            return false;
    }
    return true;
}

// 更新车辆路径根据最短路径表
void Map::updatecarpath(Car* const thecar)
{
    //更新在路上的车子路径, 不更新要到达终点的车子, 不更新快要过路口的车子
    if (thecar->state() != Car::STATE::ON_OVER || thecar->road() == thecar->roads().back() || 
        thecar->road()->length() < thecar->road()->speed() + thecar->posid() + 1)
        return;
    const vector<int>& pathcrossid = _minpathtable.at(pair<int, int>(thecar->road()->to(), thecar->to()));
    if (thecar->road()->id() == _roads.at(pair<int, int>(pathcrossid[0], pathcrossid[1]))->id()) //不更新要掉头的车子
        return;
    vector<Road*> roads;
    roads.push_back(thecar->road());
    for (size_t i = 0; i < pathcrossid.size() - 1; i++)
        roads.push_back(_roads.at(pair<int, int>(pathcrossid[i], pathcrossid[i + 1])));
    thecar->changeroads(roads);
}

// 设置车辆路径
void Map::setcarpath(Car* const thecar)
{
    const vector<int>& pathcrossid = _minpathtable.at(pair<int, int>(thecar->from(), thecar->to()));
    vector<Road*> roads;
    for (size_t i = 0; i < pathcrossid.size() - 1; i++)
        roads.push_back(_roads[pair<int, int>(pathcrossid[i], pathcrossid[i + 1])]);
    thecar->setroads(roads);
}

// 计算一辆车规划路径拥挤率，0-100, calcnumbersofpath表示考虑拥堵的前多少条路段
int Map::carpathcongestionrate(Car* const thecar, const int calcnumbersofpath) const
{
    vector<int> rates;
    for (auto&road : thecar->roads())
        rates.push_back(road->carloadingrate());
    sort(rates.begin(), rates.end(), [](const int a, const int b) -> bool { return a > b; });
    int num = min(calcnumbersofpath, static_cast<int>(rates.size()));
    int sum = 0;
    for (int i = 0; i < num; ++i)
        sum += rates[i];
    return sum / num;
}

int Map::placecarsabilityofthecross(Cross*const thecross) const
{
    int sum = 0;
    for (auto& road : thecross->outcrossroads())
        sum += road.second->speed() * road.second->channel();
    return sum;
}


//开始仿真，返回值为总调度时间，如果死锁返回INT32_MAX
int Map::run(float roadcoefficient)
{
    //复位道路
    for (auto& road : _roads)
        road.second->resetroad();

    //复位路口
    for (auto& cross: _crosss)
        cross.second->popcrossallcars();

    //复位车辆，并安排到起始路口
    for (auto& car: _cars)
    {
        car.second->resetcar();
        _crosss.at(car.second->from())->pushgarageacar(car.second);
    }
    _carnumberinmap = 0;

    //开始运行
    int clock = 0;
    while(true)
    {
        clock++;
        _carnumberofrelease = 0;
        for (auto& road : _roads)
            for (int m = 0; m < road.second->channel(); ++m)
                driveonechannelcars(road.second, m);

        int num = 0;
        while (isallcaronover() == false)
        {
            ++num;
            if (num > static_cast<int>(_crosss.size()) * 50)
                return INT32_MAX;
            for (auto& cross : _crosss)
            {
                for (auto& road: cross.second->incrossroads())
                {
                    while(true)
                    {
                        Car* firstprioritycar = getfirstprioritycar(road.second);
                        if (firstprioritycar == nullptr || iscarconficted(cross.second, road.second, firstprioritycar) == true)
                            break;
                        int oldchannelid = firstprioritycar->channelid();   //执行到这里说明存在优先级的车辆且不与其他路车子冲突
                        if (drivecartonextroad(cross.second, road.second, firstprioritycar)) //判断当前道路第一优先级车子是否通过路口驶入了下一道路
                            driveonechannelcars(road.second, oldchannelid); //如果满足则将该车道所有车子都调度一次，但也仅仅处理在该车道行驶之后能成为over的车子
                    }
                }
            }
        }

        // 实时更新最优路径表
        updateminpathtable(roadcoefficient);

        // 更新在路上车子的路径
        for (auto& car: _cars)
            updatecarpath(car.second);

        sort(_sortedcrosss.begin(), _sortedcrosss.end(), [](Cross*const a, Cross*const b) -> bool { return a->numberofgaragecars() > b->numberofgaragecars(); });
        set<Car*> idrisecars;
        for (auto& thecross: _sortedcrosss)
        {
            if (static_cast<int>(idrisecars.size()) >= _placecarsabilityofallcrosss / 40)
                break;
            const set<Car*>& gagragecars = thecross->getgaragecars();
            vector<Car*> sortedcars;
            for (auto& thecar: gagragecars)
            {
                if (thecar->plantime() > clock)
                    continue;
                setcarpath(thecar);
                sortedcars.push_back(thecar);
            }
            
            sort(sortedcars.begin(), sortedcars.end(), [this](Car*const a, Car*const b) ->bool { return a->speed() < b->speed(); });
            for (int i = 0; i < min(placecarsabilityofthecross(thecross) / 7, static_cast<int>(sortedcars.size())); ++i)
            {
                
                if (carpathcongestionrate(sortedcars[i], 3) > 20 && _carnumberinmap > 10)
                    continue;
                
                idrisecars.insert(sortedcars[i]);
            }
        }

        for (auto& thecar: idrisecars)
        {
            if (_carnumberinmap >= _mapcapacity / 7)
                break;
            letcaronroad(thecar, _crosss[thecar->from()], clock);
        }

        // 遍历所有在路上的车子，更新车子状态
        for (auto& car : _cars)
        {
            Car* thecar = car.second;
            if (thecar->state() == Car::STATE::AT_GARAGE || thecar->state() == Car::STATE::AT_DEST)
                continue;
            thecar->setstate(Car::STATE::ON_READY);
        }

        // cout << "第" << clock << "时段运行完毕" << ",地图车辆数:" << _carnumberinmap << "当前放车数量:" << _carnumberofrelease <<endl;
        if (isallcarsatdest())
            break;
    }
    return clock;
}

#endif // !MAP_
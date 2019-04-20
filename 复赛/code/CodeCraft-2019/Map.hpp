#ifndef MAP_
#define MAP_


#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <assert.h>
#include <math.h>
#include <unordered_set>
#include <float.h>
#include "Car.hpp"
#include "Road.hpp"
#include "Cross.hpp"


using namespace std;

class Map
{
    public:
        Map() {}
        ~Map();
        void load(const string& carpath, const string& roadpath, const string& crosspath, const string& presetanswerpath);
        map<string, int> simulation(int limitedcarsnuminmap1 = 2600, int limitedcarsnuminmap2 = 4000, size_t limitedreleasecarsnum1 = 50, size_t limitedreleasecarsnum2 = 80, float roadcoefficient = 0.95);
        map<int, vector<int>> answer() const;
        map<string, int> judger(const map<int, vector<int>>& answer);
        int getcrosssnum () const { return _crosss.size(); }
        int getroadsnum() const { return _roads.size(); }
    private:
        bool letcaronroad(Car* const thecar, bool priority);
        bool isallcaronover() const;
        bool isallcarsatdest() const;
        int getstepofcarcango(Car* const thecar, Road* const theroad) const;
        void updateminpathtable(float roadcoefficient);
        void updatecarpath(Car* const thecar);
        bool iscarconficted(Cross*const thecross, Road*const theroad, Car* const thecar) const;
        void driveonechannelcars(Road* const theroad, const int channel);
        Car* getfirstprioritycar(Road*const theroad) const;
        bool drivecartonextroad(Cross* const thecross, Road*const theroad, Car* const thecar);
        bool istheroadhasthedirectioncar(Cross*const thecross, Road*const theroad, Cross::DIRECTION direction, const bool priority) const;
        void getnextroadchannelandstep(Road* const nextroad, const int nextability, int& nextchannel, int& nextstep) const;
        float letcaronroadweight(Car* const thecar) const;
        void setcarminpath(Car* const thecar);
        const vector<Car*> getreadytoroadcarssequeue(Road*const theroad, bool priority) const;
        void setcaraccordingtotheanswer(Car* const thecar, vector<int> answerpath);
        void setonroadcarsready();
        void init(bool isjudger, float roadcoefficient = 0.2, const map<int, vector<int>> answer = map<int, vector<int>>());
        void firstround();
        bool secondround();
        void thirdround();
        map<string, int> calculateschedulingtime() const;

        //以下是地图的固有属性
        map<pair<int, int>, Road*> _roads; //存放路段
        map<int, Cross*> _crosss; //存放路口
        map<int, Car*> _cars; //存放车辆
        vector<Cross*> _idrisecrosss; //存放id升序的路口, 更新路径表要用，必须是随机访问类型

        double _a = 0; //系数因子a
        double _b = 0; //系数因子b
        int _mapcapacity = 0; //地图承载车辆的能力

        //以下是地图动态信息
        map<pair<int, int>, vector<int>> _minpathtable; //最短路径查找表
        map<pair<int, int>, float> _mindisttable; //最短路径距离查找表
        set<Car*> _onroadcars; //存放在路上的车子, 便于更新在路上车子的路径
        set<Car*> _garageordinarycars; //存放车库里的普通车辆
        set<Car*> _garagepresetcars; //存放在车库里的预设车辆
        set<Car*> _garageprioritycars; //存放车库里的优先车辆
        set<Car*> _garagepresetandprioritycars; //存放车库里的预设优先车辆
        int _carnumberinmap = 0; //统计路段上的车辆数
        int _carnumberofrelease = 0; //统计当前放车的数量
        int _carnumberofarrive = 0;
        int _clock = 0; //时钟
};

// 读取car.txt, cross.txt, road.txt文件，并构造地图和车辆
void Map::load(const string& carpath, const string& roadpath, const string& crosspath, const string& presetanswerpath)
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
        int id, from, to, speed, plantime, priority, preset;
        iss >> id >> from >> to >> speed >> plantime >> priority >> preset;
        _cars.insert(pair<int, Car*>(id, new Car(id, from, to, speed, plantime, priority, preset)));
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

    //设置预置车辆路径和实际出发时间
    in.open(presetanswerpath, ifstream::in);
    while(getline(in, line))
    {
        if (line[0] == '#')
            continue;
        for (auto&i: line)
            if (i == '(' || i == ')' || i == ',')
                i = ' ';
        istringstream iss(line);
        int carid, starttime;
        
        iss >> carid >> starttime;
        vector<int> roadids;
        int roadid;
        while (iss >> roadid)
            roadids.push_back(roadid);
        vector<Road*> roads;
        int from = _cars.at(carid)->from();
        for (size_t i = 0; i < roadids.size(); i++)
        {
            int to = -1;
            for (auto& road: _roads)
            {
                if (road.first.first == from && road.second->id() == roadids[i])
                {
                    to = road.first.second;
                    break;
                }
            }
            roads.push_back(_roads.at(pair<int, int>(from, to)));
            from = to;
        }
        _cars.at(carid)->setstarttime(starttime);
        _cars.at(carid)->setroads(roads);
    }
    in.close();


    //计算系数因子a和b
    float carsnum = 0; //车辆总数
    float prioritycarsnum = 0; //优先车辆总数
    float carsminspeed = FLT_MAX; //车辆最小速度
    float carsmaxspeed = FLT_MIN; //车辆最大速度
    float prioritycarsminspeed = FLT_MAX; //优先车辆最小速度
    float prioritycarsmaxspeed = FLT_MIN; //优先车辆最大速度
    float carsearliestplantime = FLT_MAX; //最早出发时间
    float carslatestplantime = FLT_MIN; //最晚出发时间
    float prioritycarsearliestplantime = FLT_MAX; //优先车辆最早出发时间
    float prioritycarslatestplantime = FLT_MIN; //优先车辆最晚出发时间
    set<int> carsfromset; //车辆起点分布
    set<int> prioritycarsfromset; //优先车辆起点分布
    set<int> carstoset; //车辆终点分布
    set<int> prioritycarstoset; //优先车辆终点分布
    for (auto& car : _cars)
    {
        Car* thecar = car.second;
        ++carsnum;
        if (thecar->speed() < carsminspeed)
            carsminspeed = thecar->speed();
        if (thecar->speed() > carsmaxspeed)
            carsmaxspeed = thecar->speed();
        if (thecar->plantime() < carsearliestplantime)
            carsearliestplantime = thecar->plantime();
        if (thecar->plantime() > carslatestplantime)
            carslatestplantime = thecar->plantime();
        carsfromset.insert(thecar->from());
        carstoset.insert(thecar->to());
        if (thecar->priority() == false)
            continue;
        ++prioritycarsnum;
        if (thecar->speed() < prioritycarsminspeed)
            prioritycarsminspeed = thecar->speed();
        if (thecar->speed() > prioritycarsmaxspeed)
            prioritycarsmaxspeed = thecar->speed();
        if (thecar->plantime() < prioritycarsearliestplantime)
            prioritycarsearliestplantime = thecar->plantime();
        if (thecar->plantime() > prioritycarslatestplantime)
            prioritycarslatestplantime = thecar->plantime();
        prioritycarsfromset.insert(thecar->from());
        prioritycarstoset.insert(thecar->to());
    }
    float carsfromnum = carsfromset.size(); //车辆起点分布数
    float prioritycarsfromnum = prioritycarsfromset.size(); //优先车辆起点分布数
    float carstonum = carstoset.size(); //车辆终点分布数
    float prioritycarstonum = prioritycarstoset.size(); //优先车辆终点分布数
    _a = carsnum / prioritycarsnum * 0.05 + (carsmaxspeed / carsminspeed) / (prioritycarsmaxspeed / prioritycarsminspeed) * 0.2375 + 
        (carslatestplantime / carsearliestplantime) / (prioritycarslatestplantime / prioritycarsearliestplantime) * 0.2375 + 
        carsfromnum / prioritycarsfromnum * 0.2375 + carstonum / prioritycarstonum * 0.2375;
    _b = carsnum / prioritycarsnum * 0.8 + (carsmaxspeed / carsminspeed) / (prioritycarsmaxspeed / prioritycarsminspeed) * 0.05 + 
        (carslatestplantime / carsearliestplantime) / (prioritycarslatestplantime / prioritycarsearliestplantime) * 0.05 + 
        carsfromnum / prioritycarsfromnum * 0.05 + carstonum / prioritycarstonum * 0.05;
    // cout <<"_a = " << _a << " _b = " << _b << endl;
    // _a = 1.45316; _b = 8.22386;
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

// 输出答案answer
map<int, vector<int>> Map::answer() const
{
    map<int, vector<int>> res;

    for (auto& car: _cars)
    {
        if (car.second->preset() == true)
            continue;
        vector<int> line;
        Car * thecar = car.second;
        line.push_back(thecar->id());
        line.push_back(thecar->postponedplantime());
        for (auto& theroad: thecar->roads())
            line.push_back(theroad->id());
        res.insert(pair<int, vector<int>>(thecar->id(), line));
    }
    return res;
}

// 更新最短路径表
void Map::updateminpathtable(float roadcoefficient)
{
    //floyd算法计算最短路径表
    vector<vector<float>> dist =  vector<vector<float>>(_idrisecrosss.size(), vector<float>(_idrisecrosss.size(), FLT_MAX));
    vector<vector<int>> path(_idrisecrosss.size(), vector<int>(_idrisecrosss.size()));
    for (size_t i = 0; i < _idrisecrosss.size(); ++i)
    {
        for (size_t j = 0; j < _idrisecrosss.size(); ++j)
        {
            path[i][j] = j;
            if (_roads.find(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())) == _roads.end())
                continue;
            dist[i][j] = _roads.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id()))->w(roadcoefficient);
        }
    }

    for (size_t k = 0; k < _idrisecrosss.size(); ++k)
    {
        for (size_t i = 0; i < _idrisecrosss.size(); ++i)
        {
            for (size_t j = 0; j < _idrisecrosss.size(); ++j)
            {
                double s = (dist[i][k] == FLT_MAX || dist[k][j] == FLT_MAX)? FLT_MAX: dist[i][k] + dist[k][j];
                if (s < dist[i][j])
                {
                    dist[i][j] = s;
                    path[i][j] = path[i][k];
                }
            }
        }
    }

    //更新最短路径表, 和距离表
    _minpathtable.clear();
    // _mindisttable.clear();
    for (size_t i = 0; i < _idrisecrosss.size(); ++i)
    {
        for (size_t j = 0; j < _idrisecrosss.size(); ++j)
        {
            if (i == j)
                continue;
            _minpathtable[pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())].push_back(_idrisecrosss[i]->id());
            // _mindisttable[pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())] = dist[i][j];
            size_t temp = path[i][j];
            while (temp != j)
            {
                _minpathtable.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())).push_back(_idrisecrosss[temp]->id());
                temp = path[temp][j];
            }
            _minpathtable.at(pair<int, int>(_idrisecrosss[i]->id(), _idrisecrosss[j]->id())).push_back(_idrisecrosss[j]->id());
        }
    }

    // //归一化最短距离表
    // float maxdist = max_element(_mindisttable.begin(), _mindisttable.end(), [](const pair<pair<int, int>, float>& a, const pair<pair<int, int>, float>& b) -> bool 
    // { 
    //     return a.second < b.second; 
    // })->second;
    // float mindist = min_element(_mindisttable.begin(), _mindisttable.end(), [](const pair<pair<int, int>, float>& a, const pair<pair<int, int>, float>& b) -> bool 
    // { 
    //     return a.second < b.second; 
    // })->second;
    // for (auto& d : _mindisttable)
    //     d.second = (d.second - mindist) / (maxdist - mindist);
}

// 判断车辆是否都终止运行
bool Map::isallcaronover() const
{
    for (auto&car: _onroadcars)
    {
        if (car->state() == Car::STATE::ON_WAIT)
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

// 判断这一条道路有没有这一方向的车子过路口，priority == true时只判断优先车辆, priority == false对所有车辆都适用
bool Map::istheroadhasthedirectioncar(Cross* const thecross, Road* const theroad, Cross::DIRECTION direction, const bool priority) const
{
    Car* thecar = getfirstprioritycar(theroad);
    if (thecar == nullptr || (priority == true && thecar->priority() != true))
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
    //int nextability = min(thenextroad->speed(), thecar->speed()) - step;
    if (thecar->posid() + step + 1 < theroad->length() || speed == step /*|| nextability < 1*/)
        return false; //没有以上条件就不会过路口
    if (thecross->judgedirection(theroad->id(), thenextroad->id()) == direction) //是这一方向的
        return true; //有位置
    return false;
}

// 移动theroad的channel通道的所有车，第一步和第二步都会调用
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
        else //当前车道有阻挡且前车为over或者当前道路无阻挡行驶最大距离
        {
            thecar->setstate(Car::STATE::ON_OVER);
            thecar->setposid(pos + step);
            theroad->setposnocar(channel, pos);
            theroad->setposacar(channel, pos + step, thecar);
            continue;
        }
    }
}

// 判断第一优先级的车辆是否矛盾吗
bool Map::iscarconficted(Cross* const thecross, Road* const theroad, Car* const thecar) const
{
    int step = getstepofcarcango(thecar, theroad);
    int speed = min(thecar->speed(), theroad->speed());
    if (thecar->posid() + step + 1 < theroad->length() || speed == step)
        return false; //没有以上条件就不会过路口，不矛盾
    Road* thenextroad = thecar->getnextroad();
    Road* theleftroad = nullptr, * thefrontroad = nullptr, * therightroad = nullptr;
    thecross->getotherdirectionincrossroads(theroad->id(), theleftroad, thefrontroad, therightroad);
    if (thenextroad == nullptr)
    {
        if (thecar->priority() == true)
            return false;
        else
        {
            //此处到达终点的车辆不是优先车辆，需判断左边有没有左转的优先车辆，还要判断右边道路有没有右转优先车辆
            if ((theleftroad != nullptr && istheroadhasthedirectioncar(thecross, theleftroad, Cross::DIRECTION::LEFT, true) == true) ||
                (therightroad != nullptr && istheroadhasthedirectioncar(thecross, therightroad, Cross::DIRECTION::RIGHT, true) == true))
                return true;
            else
                return false;
        }
    }
    if (min(thenextroad->speed(), thecar->speed()) - step < 1)
        return false; //没有以上条件就不会过路口，不矛盾
    if (thecross->judgedirection(theroad->id(), thenextroad->id()) == Cross::DIRECTION::STRAIGHT)
    {
        if (thecar->priority() == true)
            return false;
        else
        {
            //此处直行的车辆不是优先车辆，需判断左边有没有左转的优先车辆，还要判断右边道路有没有右转优先车辆
            if ((theleftroad != nullptr && istheroadhasthedirectioncar(thecross, theleftroad, Cross::DIRECTION::LEFT, true) == true) || 
                (therightroad != nullptr && istheroadhasthedirectioncar(thecross, therightroad, Cross::DIRECTION::RIGHT, true) == true))
                return true;
            else
                return false;
        }
    }
    else if (thecross->judgedirection(theroad->id(), thenextroad->id()) == Cross::DIRECTION::LEFT)
    {
        if (thecar->priority() == true)
        {
            //该优先车辆要左转，需要判断右边道路是否有优先车辆要直行
            if (therightroad != nullptr && istheroadhasthedirectioncar(thecross, therightroad, Cross::DIRECTION::STRAIGHT, true) == true)
                return true;
            else
                return false;
        }
        else
        {
            //该车辆不是优先车辆，需要左转，要判断右方路段是否有直行车辆，要判断前方路段是否有优先车辆右转
            if ((therightroad != nullptr && istheroadhasthedirectioncar(thecross, therightroad, Cross::DIRECTION::STRAIGHT, false) == true) || 
                (thefrontroad != nullptr && istheroadhasthedirectioncar(thecross, thefrontroad, Cross::DIRECTION::RIGHT, true) == true))
                return true;
            else
                return false;
        }
    }
    else if (thecross->judgedirection(theroad->id(), thenextroad->id()) == Cross::DIRECTION::RIGHT)
    {
        if (thecar->priority() == true)
        {
            //该优先车辆要右转，需要判断左边道路是否有直行的优先车辆，还有有前方路段是否有左转优先车辆
            if ((theleftroad != nullptr && istheroadhasthedirectioncar(thecross, theleftroad, Cross::DIRECTION::STRAIGHT, true) == true) || 
                (thefrontroad != nullptr && istheroadhasthedirectioncar(thecross, thefrontroad, Cross::DIRECTION::LEFT, true) == true))
                return true;
            else
                return false;
        }
        else
        {
            //该车辆不是优先车辆，优先级别最低，需要右转，要判断左方路段是否有直行车辆，要判断前方路段是否有左转
            if ((theleftroad != nullptr && istheroadhasthedirectioncar(thecross, theleftroad, Cross::DIRECTION::STRAIGHT, false) == true) ||
                (thefrontroad != nullptr && istheroadhasthedirectioncar(thecross, thefrontroad, Cross::DIRECTION::LEFT, false) == true))
                return true;
            else
                return false;
        }
    }
    throw;
}

// 将车辆移动到下一路口
bool Map::drivecartonextroad(Cross* const thecross, Road* const theroad, Car* const thecar)
{
    int oldposid = thecar->posid();
    int oldchannelid = thecar->channelid();
    int step = getstepofcarcango(thecar, theroad);
    Road* thenextroad = thecar->getnextroad();
    if (thenextroad == nullptr)
    {
        thecar->setstate(Car::STATE::AT_DEST);
        thecar->setposid(0);
        thecar->setchannelid(0);
        thecar->setroad(nullptr);
        thecar->setdesttime(_clock);
        theroad->setposnocar(oldchannelid, oldposid);
        --_carnumberinmap;
        ++_carnumberofarrive;
        _onroadcars.erase(thecar);
        return true;
    }
    int nextability = min(thenextroad->speed(), thecar->speed()) - step;
    if (nextability < 1)
    {
        //步数不够车子还不能转, 在这条道最前面终止
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(oldposid + step);
        theroad->setposnocar(oldchannelid, oldposid);
        theroad->setposacar(oldchannelid, oldposid + step, thecar);
        return true;
    }
    int nextchannel = 0;
    int nextstep = 0;
    getnextroadchannelandstep(thenextroad, nextability, nextchannel, nextstep);

    if (nextstep == 0)
    {
        // 堵死扫描最后面的车子，状态
        for (int i = 0; i < thenextroad->channel(); ++i)
            if (thenextroad->getcarfrompos(i, 0)->state() == Car::STATE::ON_WAIT)
                return false;
    }
    //前车是wait
    if (0 < nextstep && nextstep < nextability && thenextroad->getcarfrompos(nextchannel, nextstep)->state() == Car::STATE::ON_WAIT)
        return false;
    //低通道最后一辆车是wait
    if (nextchannel > 0)
    {
        for (int i = 0; i < nextchannel; ++i)
            if (thenextroad->getcarfrompos(i, 0)->state() == Car::STATE::ON_WAIT)
                return false;
    }
    if (nextstep == 0)
    {
        //下一条路太堵，只能在道路的最前方终止
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(oldposid + step);
        theroad->setposnocar(oldchannelid, oldposid);
        theroad->setposacar(oldchannelid, oldposid + step, thecar);
        return true;
    }
    else
    {
        //过路口了
        thecar->setstate(Car::STATE::ON_OVER);
        thecar->setposid(nextstep - 1);
        thecar->setchannelid(nextchannel);
        thecar->gocrossnumplusone();
        theroad->setposnocar(oldchannelid, oldposid);
        thenextroad->setposacar(nextchannel, nextstep - 1, thecar);
        thecar->setroad(thenextroad);
        return true;
    }
    throw;
}

// 得到该道路的第一优先级车辆
Car* Map::getfirstprioritycar(Road*const theroad) const
{
    //保存每一通道的前列车辆，如果该通道无车则前列车辆为pair<int, Car*>(-1, nullptr)，pair的第一个元素表示车子所在的位置
    vector<pair<int, Car*>> frontcars(theroad->channel(), pair<int, Car*>(-1, nullptr));
    for (int i = 0; i < theroad->channel(); ++i)
    {
        for (int j = theroad->length() - 1; j >= 0; --j)
        {
            if (theroad->poshasacar(i, j) == false || theroad->getcarfrompos(i, j)->state() == Car::STATE::ON_OVER) //此位置没有车或者车子处于over状态，处于over状态的车子不参与优先级排序
                continue;
            frontcars[i] = pair<int, Car*>(j, theroad->getcarfrompos(i, j));
            break;
        }
    }

    //如果有优先车辆，首先返回
    pair<int, Car*> firstprioritycar = pair<int, Car*>(-1, nullptr);
    for (auto& p: frontcars)
    {
        if (p.first == -1)
            continue;
        if (p.second->priority() == true && p.first > firstprioritycar.first)
            firstprioritycar = p;
    }
    if (firstprioritycar.first != -1)
        return firstprioritycar.second;

    //如果没有优先车辆，则返回位置最前的，通道相对小的车辆
    for (auto& p: frontcars)
    {
        if (p.first == -1)
            continue;
        if (p.first > firstprioritycar.first)
            firstprioritycar = p;
    }
    return firstprioritycar.second;
}

// 设置该条道路，准备上路车辆的优先级队列, priority == true时，设置的都是车库里的优先车辆， priority == false时，设置车库的所有车辆
const vector<Car*> Map::getreadytoroadcarssequeue(Road*const theroad, bool priority) const
{
    const set<Car*>& garagecars = theroad->getgaragecars();
    vector<Car*> carssequeue;
    for (auto& thecar : garagecars)
    {
        if ((thecar->preset() ? thecar->starttime() : thecar->postponedplantime()) > _clock || (priority == true && thecar->priority() != true))
            continue;
        carssequeue.push_back(thecar);
    }
    sort(carssequeue.begin(), carssequeue.end(), [](Car*const a, Car* const b) ->bool 
    {
        //首先按照优先车辆排序，然后按照时间排序，最后按照id排序
        if (a->priority() == true && b->priority() == false)
            return true;
        else if (a->priority() == false && b->priority() == true)
            return false;
        else
        {
            //同时都是优先车辆，或者同时都不是优先车辆
            int timea = a->preset() ? a->starttime() : a->postponedplantime();
            int timeb = b->preset() ? b->starttime() : b->postponedplantime();
            if (timea < timeb)
                return true;
            else if (timea > timeb)
                return false;
            else
                return a->id() < b->id();
        }
    });
    return carssequeue;
}

// 让车辆上路
bool Map::letcaronroad(Car* const thecar, bool priority)
{
    assert(thecar->preset() == true ? thecar->starttime() <= _clock : thecar->plantime() <= _clock);

    // if (thecar->id() == 48905)
    //     cout << "aasd" << endl;

    Road* thenextroad = thecar->getnextroad();
    int nextability = min(thecar->speed(), thenextroad->speed());
    int nextchannel = 0, nextstep = 0;
    getnextroadchannelandstep(thenextroad, nextability, nextchannel, nextstep);

    if (priority == false)
    {
        if (nextstep == 0)
            return false;
    }
    else
    {
        if (nextstep == 0)
            return false; //没空位，挤不进去
        
        if (0 < nextstep && nextstep < nextability && thenextroad->getcarfrompos(nextchannel, nextstep)->state() == Car::STATE::ON_WAIT)
            return false; //有空位，前车产生阻挡，要判断前车什么状态

        if (nextchannel > 0)
        {
            for (int i = 0; i < nextchannel; ++i)
                if (thenextroad->getcarfrompos(i, 0)->state() == Car::STATE::ON_WAIT)
                    return false;
        }
    }
    thenextroad->setposacar(nextchannel, nextstep - 1, thecar);
    thecar->setchannelid(nextchannel);
    thecar->setposid(nextstep - 1);
    thecar->setstate(Car::STATE::ON_OVER);
    thecar->setroad(thenextroad);
    if (thecar->preset() == false)
    {
        thecar->setstarttime(_clock);
        if (thecar->priority() == false)
            _garageordinarycars.erase(thecar);
        
        else
            _garageprioritycars.erase(thecar);
    }
    else
    {
        if (thecar->priority() == false)
            _garagepresetcars.erase(thecar);
        else
            _garagepresetandprioritycars.erase(thecar);
    }
    ++_carnumberinmap;
    ++_carnumberofrelease;
    _onroadcars.insert(thecar);
    return true;
}

// 判断是不是所有车子到达终点
bool Map::isallcarsatdest() const
{
    if (_onroadcars.size() > 0 || 
        _garageordinarycars.size() > 0 || 
        _garageprioritycars.size() > 0 || 
        _garagepresetcars.size() > 0 || 
        _garagepresetandprioritycars.size() > 0)
        return false;
    return true;
}

// 更新车辆路径根据最短路径表,只能对非预设车辆
void Map::updatecarpath(Car* const thecar)
{
    assert(thecar->preset() == false);
    //更新在路上的车子路径, 不更新要到达终点的车子, 不更新快要过路口的车子
    if (thecar->state() != Car::STATE::ON_READY || thecar->road() == thecar->roads().back() || 
        thecar->road()->length() < min(thecar->road()->speed(), thecar->speed()) + thecar->posid() + 1)
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

// 设置车辆路径,只能对非预设车辆
void Map::setcarminpath(Car* const thecar)
{
    assert(thecar->preset() == false);
    const vector<int>& pathcrossid = _minpathtable.at(pair<int, int>(thecar->from(), thecar->to()));
    vector<Road*> roads;
    for (size_t i = 0; i < pathcrossid.size() - 1; i++)
        roads.push_back(_roads.at(pair<int, int>(pathcrossid[i], pathcrossid[i + 1])));
    thecar->setroads(roads);
}

// 计算一辆车上路权重
float Map::letcaronroadweight(Car* const thecar) const
{
    vector<float> rates;
    for (auto&road : thecar->roads())
        rates.push_back(road->carloadingrate());
    float sum = 0;
    size_t num = rates.size();
    for (size_t i = 0; i < num; ++i)
        sum += pow<float, float>(rates[i], 0.7 + 0.3 * (i + 1) / num);
    return sum / num;
}

//讲所有路上车辆的车辆设置为等待状态
void Map::setonroadcarsready()
{
    // 遍历所有在路上的车子，更新车子状态
    for (auto& thecar : _onroadcars)
        thecar->setstate(Car::STATE::ON_READY);
}

//sinmulation()和judger()的初始化程序
void Map::init(bool isjudger, float roadcoefficient, const map<int, vector<int>> answer)
{
    //复位道路
    for (auto& road : _roads)
        road.second->resetroad();

    //计算初始的路径表和距离表, 判题器运行时就不用了
    if (isjudger == false)
        updateminpathtable(roadcoefficient);

    //复位车辆，并安排到起始道路的初始列表当中
    _onroadcars.clear();
    _garageordinarycars.clear();
    _garagepresetcars.clear();
    _garageprioritycars.clear();
    _garagepresetandprioritycars.clear();
    for (auto& car: _cars)
    {
        Car* thecar = car.second;
        thecar->resetcar();
        if (thecar->preset() == false && thecar->priority() == false) //普通车辆
        {
            if (isjudger == false)
                setcarminpath(thecar); //仿真器用最短路径生成路径
            else
                setcaraccordingtotheanswer(thecar, answer.at(thecar->id())); //判题器读取，答案的路径
            _garageordinarycars.insert(thecar);
        }
        else if (thecar->preset() == true && thecar->priority() == false) //预设车辆
            _garagepresetcars.insert(thecar);
        else if (thecar->preset() == false && thecar->priority() == true) //优先车辆
        {
            if (isjudger == false)
                setcarminpath(thecar); //仿真器用最短路径生成路径
            else
                setcaraccordingtotheanswer(thecar, answer.at(thecar->id())); //判题器读取，答案的路径
            _garageprioritycars.insert(thecar);
        }
        else //预设优先车辆
            _garagepresetandprioritycars.insert(thecar);
        Road* thenextroad = thecar->getnextroad();
        thenextroad->pushcarintogarage(thecar); //将车放到起始道路的车辆队列当中
    }
    _carnumberinmap = 0;
    _carnumberofarrive = 0;
    _clock = 0;
}

//第一轮
void Map::firstround()
{
    for (auto& road : _roads)
        for (int m = 0; m < road.second->channel(); ++m)
            driveonechannelcars(road.second, m);

    //让道路里准备上路的车辆上路
    for (auto& road : _roads)
    {
        for (auto& thecar : getreadytoroadcarssequeue(road.second, true))
        {
            if (letcaronroad(thecar, true))
                road.second->popcarfromgarage(thecar);
        }

    }
}

//第二轮，返回是否死锁
bool Map::secondround()
{
    //第二步
    int num = 0;
    while (isallcaronover() == false)
    {
        ++num;
        if (num > static_cast<int>(_crosss.size()) * 50)
            return true;
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
                    if (drivecartonextroad(cross.second, road.second, firstprioritycar))
                    {
                        driveonechannelcars(road.second, oldchannelid); //对第一优先级后面的所有车辆做一次推动
                        for (auto& thecar : getreadytoroadcarssequeue(road.second, true)) //按照次序让优先车辆上路
                        {
                            if (letcaronroad(thecar, true))
                                road.second->popcarfromgarage(thecar);
                        }
                    }
                    else
                        break;
                }
            }
        }
    }
    return false;
}

//第三轮
void Map::thirdround()
{
    //让车子上路
    for (auto&road : _roads)
    {
        for (auto& thecar : getreadytoroadcarssequeue(road.second, false))
        {
            if (letcaronroad(thecar, false))
                road.second->popcarfromgarage(thecar);
        }

    }
    // 遍历所有在路上的车子，设置ready状态
    setonroadcarsready();
}

//根据运行结果计算调度时间
map<string, int> Map::calculateschedulingtime() const
{  
    float originscheduletime = _clock; //原调度时间
    float originallscheduletime = 0; //原车辆调度时间
    float specialscheduletime = 0; //优先调度时间
    float specialallscheduletime = 0; //优先车辆调度时间
    float endscheduletime = 0; //最终调度时间
    float endallscheduletime = 0; //最终车辆调度时间

    float prioritycarsminplantime = FLT_MAX;
    float prioritycarsmaxdesttime = 0;

    for (auto& car : _cars)
    {
        Car* thecar = car.second;
        originallscheduletime += thecar->desttime() - thecar->plantime();
        if (thecar->priority() == false)
            continue;
        if (thecar->plantime() < prioritycarsminplantime)
            prioritycarsminplantime = thecar->plantime();
        if (thecar->desttime() > prioritycarsmaxdesttime)
            prioritycarsmaxdesttime = thecar->desttime();
        specialallscheduletime += thecar->desttime() - thecar->plantime();
    }
    specialscheduletime = prioritycarsmaxdesttime - prioritycarsminplantime;
    endscheduletime = _a * specialscheduletime + originscheduletime;
    endallscheduletime = _b * specialallscheduletime + originallscheduletime;
    map<string, int> res;
    res["originscheduletime"] = originscheduletime;
    res["originallscheduletime"] = originallscheduletime;
    res["specialscheduletime"] = specialscheduletime;
    res["specialallscheduletime"] = specialallscheduletime;
    res["endscheduletime"] = endscheduletime;
    res["endallscheduletime"] = endallscheduletime;
    return res;
}

// 开始仿真，返回值为总调度时间，如果死锁返回空字典
map<string, int> Map::simulation(int limitedcarsnuminmap1, int limitedcarsnuminmap2, size_t limitedreleasecarsnum1, size_t limitedreleasecarsnum2, float roadcoefficient)
{
    init(false, roadcoefficient);

    //开始运行
    while(true)
    {
        ++_clock;
        _carnumberofrelease = 0;

        //半实时更新路径
        if (_clock % 25 == 0)
        {
            // 实时更新最优路径表
            updateminpathtable(roadcoefficient);

            // 更新在路上车子的路径
            for (auto& thecar: _onroadcars)
            {
                if (thecar->preset() == false)
                    updatecarpath(thecar);
            }

            // 更新在车库当中车子的路径
            if (_clock % 50 == 0)
            {
                for (Car* thecar : _garageordinarycars)
                {
                    if (thecar->postponedplantime() < _clock)
                        continue; //小于时钟的车辆已经参与优先级队列的排序了，不能再更改它的路径
                    thecar->getnextroad()->popcarfromgarage(thecar);
                    setcarminpath(thecar);
                    thecar->getnextroad()->pushcarintogarage(thecar);
                }
                for (Car* thecar : _garageprioritycars)
                {
                    if (thecar->postponedplantime() < _clock)
                        continue; //小于时钟的车辆已经参与优先级队列的排序了，不能再更改它的路径
                    thecar->getnextroad()->popcarfromgarage(thecar);
                    setcarminpath(thecar);
                    thecar->getnextroad()->pushcarintogarage(thecar);
                }
            }
        }

        if (_carnumberinmap >= limitedcarsnuminmap1 * 0.88 + limitedcarsnuminmap1 * 0.15 * (_clock % 4) / 4)
        {
            for (auto& thecar : _garageprioritycars)
                if (thecar->postponedplantime() >= _clock)
                    thecar->postponedplantimeplus(1);
        }
        else
        {
            vector<Car*> sortedcars;
            for (auto& thecar : _garageprioritycars)
            {
                if (thecar->postponedplantime() != _clock)
                    continue;
                sortedcars.push_back(thecar);
                thecar->setletcaronroadweight(letcaronroadweight(thecar));
            }
            sort(sortedcars.begin(), sortedcars.end(), [](Car*const a, Car* const b) -> bool 
            {
                return a->letcaronroadweight() < b->letcaronroadweight();
            });
            if (limitedreleasecarsnum1 < sortedcars.size())
                for (size_t i = limitedreleasecarsnum1; i < sortedcars.size(); ++i)
                    sortedcars[i]->postponedplantimeplus(1);
        }

        int t1 = (_garageprioritycars.size() == 0 && _garagepresetandprioritycars.size() == 0 && _garagepresetcars.size() == 0) ? limitedcarsnuminmap2 : limitedcarsnuminmap1;
        size_t t2 = (_garageprioritycars.size() == 0 && _garagepresetandprioritycars.size() == 0 && _garagepresetcars.size() == 0) ? limitedreleasecarsnum2: limitedreleasecarsnum1;

        if (_carnumberinmap >= 0.8 * t1  + 0.3 * (_clock % 4) / 4 * t1)
        {
            for (auto& thecar : _garageordinarycars)
                if (thecar->postponedplantime() >= _clock)
                    thecar->postponedplantimeplus(1);
        }
        else
        {
            vector<Car*> sortedcars;
            for (auto& thecar : _garageordinarycars)
            {
                if (thecar->postponedplantime() != _clock)
                    continue;
                sortedcars.push_back(thecar);
                if (sortedcars.size() <= 4000)
                    thecar->setletcaronroadweight(letcaronroadweight(thecar));
            }
            sort(sortedcars.begin(), sortedcars.begin() + min(3999UL, sortedcars.size()), [](Car*const a, Car* const b) -> bool 
            {
                return a->letcaronroadweight() < b->letcaronroadweight();
            });
            if (t2 < sortedcars.size())
                for (size_t i = t2; i < sortedcars.size(); ++i)
                    sortedcars[i]->postponedplantimeplus(1);
        }

        //第一轮
        firstround();

        //第二轮
        if (secondround())
            return map<string, int>();

        //第三轮
        thirdround();

        cout << "第" << _clock << "时段完";
        cout << ", 车库车辆: " << _garageordinarycars.size() << " " << _garageprioritycars.size() << " " << _garagepresetandprioritycars.size() << " " << _garagepresetcars.size();
        cout << ", 地图车辆: " << _carnumberinmap;
        cout << ", 放车量: " << _carnumberofrelease << endl;
        // cout << ", 到达车辆数: " << _carnumberofarrive << endl;
        if (isallcarsatdest())
            break;
    }

    return calculateschedulingtime();
}

// 判题器程序，读取本地答案计算调度时间，如果死锁返回空字典
map<string, int> Map::judger(const map<int, vector<int>>& answer)
{
    init(true, 0, answer);

    //开始运行

    while(true)
    {
        ++_clock;
        _carnumberofrelease = 0;

        //第一轮
        firstround();

        //第二轮
        if (secondround())
            return map<string, int>();

        //第三轮
        thirdround();

        // cout << "第" << _clock << "时段完";
        // cout << ", 车库车辆: " << _garageordinarycars.size() << " " << _garageprioritycars.size() << " " << _garagepresetandprioritycars.size() << " " << _garagepresetcars.size();
        // cout << ", 地图车辆: " << _carnumberinmap;
        // cout << ", 放车量: " << _carnumberofrelease << endl;

        //判断是否所有车辆都到达终点
        if (isallcarsatdest())
            break;
    }
    return calculateschedulingtime();
}

//判题器调用，设定非预设车辆的推迟计划时间和路径
void Map::setcaraccordingtotheanswer(Car* const thecar, vector<int> answerpath)
{
    assert(thecar->id() == answerpath[0]);
    assert(thecar->preset() == false);
    vector<Road*> roads;
    int from = _cars.at(thecar->id())->from();
    for (size_t i = 2; i < answerpath.size(); i++)
    {
        int to = -1;
        for (auto& road: _roads)
        {
            if (road.first.first == from && road.second->id() == answerpath[i])
            {
                to = road.first.second;
                break;
            }
        }
        roads.push_back(_roads.at(pair<int, int>(from, to)));
        from = to;
    }
    thecar->setroads(roads);
    thecar->setpostponedplantime(answerpath[1]);
}

#endif // !MAP_
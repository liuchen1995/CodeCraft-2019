#ifndef Car_
#define Car_

#include <iostream>
#include <vector>
#include <algorithm>
#include "Road.hpp"


using namespace std;

class Car
{
    public:
        enum STATE {AT_GARAGE, ON_READY, ON_WAIT, ON_OVER, AT_DEST};
        explicit Car(int id, int from, int to, int speed, int plantime, bool priority, bool preset)
        :_id(id), _from(from), _to(to), _speed(speed), _plantime(plantime), _priority(priority), _preset(preset) {}

        int id() const { return _id; }
        int from() const { return _from; }
        int to() const { return _to; }
        int speed() const { return _speed; }
        int plantime() const { return _plantime; }
        bool priority() const { return _priority; }
        bool preset() const { return _preset; }

        operator int() const { return _id; }
        bool operator==(const Car& rhs) { return _id == rhs.id(); }

        int postponedplantime() const { return _postponedplantime; }
        void setpostponedplantime(const int postponedplantime);
        void postponedplantimeplus(const int num);
        int starttime() const { return _starttime; }
        void setstarttime(const int starttime);
        int desttime() const { return _desttime; }
        void setdesttime(const int desttime) { _desttime = desttime; }

        void gocrossnumplusone() { ++_gocrossnum; }

        Road* road() const { return _road; }
        int channelid() const { return _channelid; }
        int posid() const { return _posid; }
        STATE state() const { return _state; }
        void setstate(const STATE state) { _state = state; }
        void setchannelid(const int channelid) { _channelid = channelid; }
        void setposid(const int posid) { _posid = posid; }
        void setroad(Road* const road) { _road = road; }

        void setroads(const vector<Road*>& roads) { _roads = roads; }
        void changeroads(const vector<Road*>& newroads);
        const vector<Road*>& roads() const { return _roads; }
        Road* getnextroad() const;
        void setletcaronroadweight(const float letcaronroadweight) { _letcaronroadweight = letcaronroadweight; }
        const float letcaronroadweight() const { return _letcaronroadweight; }
        void resetcar();



    private:

        //以下车辆固有属性
        const int _id = 0; //汽车id
        const int _from = 0; //汽车起点路口
        const int _to = 0; //汽车终点路口
        const int _speed = 0; //汽车最高速度
        const int _plantime = 0; //原设计划出发时间
        const bool _priority = false; //是否是优先车辆
        const bool _preset = false; //是否是预设车辆

        //以下车辆动态属性
        int _starttime = 0; //实际出发时间
        int _postponedplantime = _plantime; //被推迟的计划时间，只对非预设车辆使用，被推迟的计划时间一定大于等于原设的计划时间

        int _desttime = 0; //到达终点的时间

        float _letcaronroadweight = 0;
        vector<Road*> _roads; //出发路径, 里面记录道路指针
        Road* _road = nullptr; //当前路段指针
        int _channelid = 0; //当前路段通道
        int _posid = 0; //当前通道前位置
        STATE _state = STATE::AT_GARAGE; //汽车状态
        size_t _gocrossnum = 0; //记录了车子过了几个路口
};

void Car::postponedplantimeplus(int num)
{
    _postponedplantime += num;
}

void Car::setpostponedplantime(const int postponedplantime)
{
    assert(this->preset() == false); //只对非预设车辆使用
    assert(postponedplantime >= _postponedplantime);
    _postponedplantime = postponedplantime;
}

void Car::setstarttime(const int starttime)
{
    assert(_starttime == 0);
    _starttime = starttime;
}

void Car::resetcar()
{
    _gocrossnum = 0;
    _channelid = 0;
    _posid = 0;
    _road = nullptr;
    _state = STATE::AT_GARAGE;
    _desttime = 0;
    if (_preset == false)
    {
        _roads.clear();
        _starttime = 0;
        _postponedplantime = _plantime;
    }
}

// 改变车辆路径,newroads的第一个元素必须是该车辆的当前道路指针
void Car::changeroads(const vector<Road*>& newroads)
{
    auto it = _roads.end();
    while (*--it != newroads[0]);
    _roads.erase(it, _roads.end());
    _roads.insert(_roads.end(), newroads.begin(), newroads.end());
}


//得到车辆的下一条路径的指针，后向查找
Road* Car::getnextroad() const
{
    if (_state == STATE::AT_GARAGE)
        return _roads[0]; 
    if (_state == STATE::ON_READY || _state == STATE::ON_WAIT || _state == STATE::ON_OVER)
    {
        if (_gocrossnum + 1 == _roads.size())
            return nullptr;
        return _roads[_gocrossnum + 1];
    }
    throw;
}


#endif // !Car_
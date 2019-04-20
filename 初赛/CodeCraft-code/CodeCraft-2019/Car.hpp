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
        explicit Car(int id, int from, int to, int speed, int plantime)
        :_id(id), _from(from), _to(to), _speed(speed), _plantime(plantime){}

        int id() const { return _id; }
        int from() const { return _from; }
        int to() const { return _to; }
        int speed() const { return _speed; }
        int plantime() const { return _plantime; }
        int starttime() const { return _starttime; }
        int posid() const { return _posid; }
        Road* road() const { return _road; }
        int channelid() const { return _channelid; }
        STATE state() const { return _state; }
        const vector<Road*>& roads() const { return _roads; }

        Road* getnextroad() const;

        void resetcar();
        void setstarttime(const int starttime) { _starttime = starttime; }
        void setroads(const vector<Road*>& roads) { _roads = roads; }
        void changeroads(const vector<Road*>& newroads);
        void setstate(const STATE state) { _state = state; }
        void setchannelid(const int channelid) { _channelid = channelid; }
        void setposid(const int posid) { _posid = posid; }
        void setroad(Road* const road) { _road = road; }
        operator int() const { return _id; }
        bool operator==(const Car& rhs) { return _id == rhs.id(); }

    private:
        const int _id = 0; //汽车id
        const int _from = 0; //汽车起点路口
        const int _to = 0; //汽车终点路口
        const int _speed = 0; //汽车最高速度
        const int _plantime = 0; //计划出发时间

        int _starttime = 0; //实际出发时间
        vector<Road*> _roads; //出发路径, 里面记录道路指针

        int _channelid = 0; //当前路段通道数
        int _posid = 0; //当前路段前
        Road* _road = nullptr; //当前路段指针
        STATE _state = STATE::AT_GARAGE; //汽车状态
};

void Car::resetcar()
{
    _starttime = 0;
    _roads.clear();
    _channelid = 0;
    _posid = 0;
    _road = nullptr;
    _state = STATE::AT_GARAGE;
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
        auto it = _roads.end();
        while (*--it != _road);
        ++it;
        if (it == _roads.end())
            return nullptr;
        return *it;
    }
    throw;
    return nullptr;
}

#endif // !Car_
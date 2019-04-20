#ifndef Road_
#define Road_

#include <iostream>
#include <vector>
#include <math.h>
#include "Car.hpp"
#include "Cross.hpp"

using namespace std;

class Car;

class Road
{
    public:
        explicit Road(int id, int length, int speed, int channel, int from, int to)
        :_id(id), _length(length), _speed(speed), _channel(channel), _from(from), _to(to)
        , _conditions(_channel, vector<Car*>(_length, nullptr)) {}

        int id() const { return _id; }
        int length() const { return _length; }
        int speed() const { return _speed; }
        int channel() const { return _channel; }
        int from() const { return _from; }
        int to() const { return _to; }
        int w(int roadcoefficient) const;
        int carloadingrate() const;

        bool poshasacar(const int channelid, const int posid) const { return _conditions[channelid][posid] != nullptr; }
        void setposacar(const int channelid, const int posid, Car* const car);
        void setposnocar(const int channelid, const int posid);

        void resetroad();

        Car* getcarfrompos(const int channelid,const int posid) { return _conditions[channelid][posid]; }

        int currentnumberofcar() const { return _carnum; }
        operator int()  { return _id; }
        bool operator==(const Road& rhs) { return _id == rhs.id(); }

    private:
        const int _id = 0; //路段
        const int _length = 0; //路段长度
        const int _speed = 0; //路段限速
        const int _channel = 0; //通道数
        const int _from = 0; //起点路口id
        const int _to = 0; //终点路口id

        int _carnum = 0; //记录该路段有多少车子
        vector<vector<Car*>> _conditions; //路段结构
};

// 这个权值用来做路径规划的
int Road::w(int roadcoefficient) const
{
    return 100 * _length / _speed / pow(_channel, 0.5) * _length * _channel / (roadcoefficient * _length * _channel - _carnum);
}

// 道路的载车率0-100, 0表示该道路无车，100表示该道路满载
int Road::carloadingrate() const
{
    return 100 * _carnum / _length / _channel;
}

void Road::resetroad()
{
    _carnum = 0;
    _conditions = vector<vector<Car*>>(_channel, vector<Car*>(_length, nullptr));
}

void Road::setposacar(const int channelid, const int posid, Car* const car)
{
    _conditions[channelid][posid] = car;
    ++_carnum;
}

void Road::setposnocar(const int channelid, const int posid) 
{
    _conditions[channelid][posid] = nullptr; 
    --_carnum;
}

#endif // !RoadEdge_
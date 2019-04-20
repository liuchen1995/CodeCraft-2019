#ifndef Road_
#define Road_

#include <iostream>
#include <vector>
#include <array>
#include <queue>
#include <math.h>
#include <assert.h>
#include "Car.hpp"
#include "Cross.hpp"

using namespace std;

class Car;

class Road
{
    public:
        explicit Road(int id, int length, int speed, int channel, int from, int to)
        :_id(id), _length(length), _speed(speed), _channel(channel), _from(from), _to(to), _conditions(channel, vector<Car*>(_length, nullptr))
        , _garagecars(), _carsnumofchannels(channel, 0) {}

        int id() const { return _id; }
        int length() const { return _length; }
        int speed() const { return _speed; }
        int channel() const { return _channel; }
        int from() const { return _from; }
        int to() const { return _to; }

        operator int()  { return _id; }
        bool operator==(const Road& rhs) { return _id == rhs.id(); }

        float w(float roadcoefficient) const;
        float carloadingrate() const;

        bool poshasacar(const int channelid, const int posid) const { return _conditions[channelid][posid] != nullptr; }
        void setposacar(const int channelid, const int posid, Car* const car);
        void setposnocar(const int channelid, const int posid);
        Car* getcarfrompos(const int channelid,const int posid) { return _conditions[channelid][posid]; }

        void pushcarintogarage(Car*const thecar);
        void popcarfromgarage(Car*const thecar) { _garagecars.erase(thecar); }
        const set<Car*>& getgaragecars() const { return _garagecars; }

        void resetroad();

    private:

        //以下道路固有属性
        const int _id = 0; //路段
        const int _length = 0; //路段长度
        const int _speed = 0; //路段限速
        const int _channel = 0; //通道数
        const int _from = 0; //起点路口id
        const int _to = 0; //终点路口id
        const float _w = static_cast<float>(_length) / _speed / pow<float, float>(_channel, 0.5);

        //以下道路动态信息
        vector<vector<Car*>> _conditions; //路段结构
        set<Car*> _garagecars; //初始化的上路集合
        vector<float> _carsnumofchannels; //统计计算每一个通道的车辆数
};

void Road::pushcarintogarage(Car*const thecar)
{
    assert(_garagecars.count(thecar) == 0);
    _garagecars.insert(thecar);
}

// 这个权值用来做路径规划的
float Road::w(float roadcoefficient) const
{
    return 100 * _w * carloadingrate() * roadcoefficient + 100 * _w * (1 - roadcoefficient);
}

// 道路的载车率0-1, 0表示该道路无车，1表示该道路满载
float Road::carloadingrate() const
{
    float a = 0;
    for (auto i : _carsnumofchannels)
        a += pow<float, float>(i / _length, 0.7 + 0.3 * ((i + 1) / _channel));
    return a / _channel;
}

//复位道路
void Road::resetroad()
{
    _carsnumofchannels = vector<float>(_channel, 0);
    _conditions = vector<vector<Car*>>(_channel, vector<Car*>(_length, nullptr));
    _garagecars.clear();
}

//在此处设置一辆车
void Road::setposacar(const int channelid, const int posid, Car* const car)
{
    assert(_conditions[channelid][posid] == nullptr);
    _conditions[channelid][posid] = car;
    ++_carsnumofchannels[channelid];
}

//在此处消除一辆车
void Road::setposnocar(const int channelid, const int posid) 
{
    assert(_conditions[channelid][posid] != nullptr);
    _conditions[channelid][posid] = nullptr; 
    --_carsnumofchannels[channelid];
}

#endif // !Road_
#ifndef Cross_
#define Cross_

#include <vector>
#include <set>
#include <list>
#include <algorithm>
#include <iostream>
#include <map>
#include "Car.hpp"
#include "Road.hpp"

using namespace std;

class Road;
class Car;


class Cross
{
    public:
        enum DIRECTION {STRAIGHT = 2, LEFT = 1, RIGHT = 3, TURN = 0};
        explicit Cross(int id, int roadid0, int roadid1, int roadid2, int roadid3)
        :_id(id), _roadsid({roadid0, roadid1, roadid2, roadid3}) {}
        int id() const { return _id; }
        int numberofallcars() const { return _garage.size() + _dest.size(); }
        const set<Car*>& getgaragecars() const { return _garage; }
        int numberofgaragecars() const { return _garage.size(); }
        bool garagehascar(Car* const car) const { return _garage.count(car); }
        void pushgarageacar(Car* const car) { _garage.insert(car); }
        void popgarageacar(Car* const car);
        const set<Car*>& getdestcars() const { return _dest; }
        int numberofdestcars() const { return _dest.size(); }
        bool desthascar(Car* const car) const { return _dest.count(car); }
        void pushdestacar(Car* const car) { _dest.insert(car); }
        void popdestacar(Car* const car);
        const vector<int>& roadids() const { return _roadsid; }
        const map<int, Road*>& incrossroads() const { return _incrossroads; }
        const map<int, Road*>& outcrossroads() const { return _outcrossroads; }
        void setincrossroads(const map<int, Road*>& incrossroads) { _incrossroads = incrossroads; }
        void setoutcrossroads(const map<int, Road*>& outcrossroads) { _outcrossroads = outcrossroads; }
        DIRECTION judgedirection(const int currentroadid, const int nextroadid) const;
        void getotherdirectionincrossroads(int const currentroadid, Road*& theleftroad, Road*& thefrontroad, Road*& therightroad) const;
        operator int() const { return _id; }
        bool operator==(const Cross& rhs) const { return _id == rhs.id(); }
        void popcrossallcars();

    private:
        const int _id = 0; //路口id
        const vector<int> _roadsid; //连接道路的顺序
        map<int, Road*> _incrossroads; //入路口
        map<int, Road*> _outcrossroads; //出路口
        set<Car*> _garage; //存放待出发车辆
        set<Car*> _dest; //存放到达终点的车辆
};

void Cross::popgarageacar(Car* const car)
{
    auto it = find(_garage.begin(), _garage.end(), car);
    _garage.erase(it);
}

void Cross::popdestacar(Car* const car)
{
    auto it = find(_dest.begin(), _dest.end(), car);
    _dest.erase(it);
}

void Cross::popcrossallcars()
{
    _garage.clear();
    _dest.clear();
}

void Cross::getotherdirectionincrossroads(const int  currentroadid, Road*& theleftroad, Road*& thefrontroad, Road*& therightroad) const
{
    int a = -1;
    for (int k = 0; k < 4; ++k)
    {
        if (_roadsid[k] == currentroadid)
        {
            a = k;
            break;
        }
    }
    if (_incrossroads.find(_roadsid[(a + 3) % 4]) != _incrossroads.end())
        therightroad = _incrossroads.at(_roadsid[(a + 3) % 4]);
    if (_incrossroads.find(_roadsid[(a + 1) % 4]) != _incrossroads.end())
        theleftroad = _incrossroads.at(_roadsid[(a + 1) % 4]);
    if (_incrossroads.find(_roadsid[(a + 2) % 4]) != _incrossroads.end())
        thefrontroad = _incrossroads.at(_roadsid[(a + 2) % 4]);
}

Cross::DIRECTION Cross::judgedirection(const int currentroadid, const int nextroadid) const
{
    auto it1 = find(_roadsid.begin(), _roadsid.end(), currentroadid);
    auto it2 = find(_roadsid.begin(), _roadsid.end(), nextroadid);
    switch (it2 - it1)
    {
        case 2:
            return DIRECTION::STRAIGHT;
        case -2:
            return DIRECTION::STRAIGHT;
        case 1:
            return DIRECTION::LEFT;
        case -1:
            return DIRECTION::RIGHT;
        case 3:
            return DIRECTION::RIGHT;
        case -3:
            return DIRECTION::LEFT;
        case 0:
            return DIRECTION::TURN;
        default:
            throw;

    }
}

#endif // !Cross_
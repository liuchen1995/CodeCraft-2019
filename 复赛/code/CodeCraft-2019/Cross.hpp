#ifndef Cross_
#define Cross_

#include <vector>
#include <set>
#include <list>
#include <algorithm>
#include <iostream>
#include <map>
#include <assert.h>
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
        const vector<int>& roadids() const { return _roadsid; }
        const map<int, Road*>& incrossroads() const { return _incrossroads; }
        const map<int, Road*>& outcrossroads() const { return _outcrossroads; }
        void setincrossroads(const map<int, Road*>& incrossroads) { _incrossroads = incrossroads; }
        void setoutcrossroads(const map<int, Road*>& outcrossroads) { _outcrossroads = outcrossroads; }
        DIRECTION judgedirection(const int currentroadid, const int nextroadid) const;
        void getotherdirectionincrossroads(int const currentroadid, Road*& theleftroad, Road*& thefrontroad, Road*& therightroad) const;
        operator int() const { return _id; }
        bool operator==(const Cross& rhs) const { return _id == rhs.id(); }

    private:
        const int _id = 0; //路口id
        const vector<int> _roadsid; //连接道路的顺序
        map<int, Road*> _incrossroads; //入路口
        map<int, Road*> _outcrossroads; //出路口
};


void Cross::getotherdirectionincrossroads(const int  currentroadid, Road*& theleftroad, Road*& thefrontroad, Road*& therightroad) const
{
    int a = find(_roadsid.begin(), _roadsid.end(), currentroadid) - _roadsid.begin();
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
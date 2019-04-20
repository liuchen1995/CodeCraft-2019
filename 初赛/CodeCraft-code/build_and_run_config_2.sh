#!/bin/bash
sh build.sh
cd bin
./CodeCraft-2019 ../config_2/car.txt ../config_2/road.txt ../config_2/cross.txt ../config_2/answer.txt

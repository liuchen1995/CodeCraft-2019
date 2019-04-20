#!/bin/bash
sh build.sh
cd bin
echo "测试地图1"
./CodeCraft-2019 ../config_1/car.txt ../config_1/road.txt ../config_1/cross.txt ../config_1/answer.txt
echo "测试地图2"
./CodeCraft-2019 ../config_2/car.txt ../config_2/road.txt ../config_2/cross.txt ../config_2/answer.txt
echo "测试地图3"
./CodeCraft-2019 ../config_3/car.txt ../config_3/road.txt ../config_3/cross.txt ../config_3/answer.txt
echo "测试地图4"
./CodeCraft-2019 ../config_4/car.txt ../config_4/road.txt ../config_4/cross.txt ../config_4/answer.txt
echo "测试地图5"
./CodeCraft-2019 ../config_5/car.txt ../config_5/road.txt ../config_5/cross.txt ../config_5/answer.txt
echo "测试地图6"
./CodeCraft-2019 ../config_6/car.txt ../config_6/road.txt ../config_6/cross.txt ../config_6/answer.txt
echo "测试地图7"
./CodeCraft-2019 ../config_7/car.txt ../config_7/road.txt ../config_7/cross.txt ../config_7/answer.txt
echo "测试地图8"
./CodeCraft-2019 ../config_8/car.txt ../config_8/road.txt ../config_8/cross.txt ../config_8/answer.txt
echo "测试地图9"
./CodeCraft-2019 ../config_9/car.txt ../config_9/road.txt ../config_9/cross.txt ../config_9/answer.txt
echo "测试地图10"
./CodeCraft-2019 ../config_10/car.txt ../config_10/road.txt ../config_10/cross.txt ../config_10/answer.txt
echo "正式地图1"
./CodeCraft-2019 ../1-map-training-1/car.txt ../1-map-training-1/road.txt ../1-map-training-1/cross.txt ../1-map-training-1/answer.txt
echo "正式地图2"
./CodeCraft-2019 ../1-map-training-2/car.txt ../1-map-training-2/road.txt ../1-map-training-2/cross.txt ../1-map-training-2/answer.txt
echo "正式地图3"
./CodeCraft-2019 ../1-map-training-3/car.txt ../1-map-training-3/road.txt ../1-map-training-3/cross.txt ../1-map-training-3/answer.txt
echo "正式地图4"
./CodeCraft-2019 ../1-map-training-4/car.txt ../1-map-training-4/road.txt ../1-map-training-4/cross.txt ../1-map-training-4/answer.txt
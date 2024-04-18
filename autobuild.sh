#!/bin/bash

set -e

if [ ! -d `pwd`/build ];then
	mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

cd `pwd`/build && 
	cmake .. &&
	make -j8

#回到根目录
cd ..

#把头文件拷贝到/usr/local/include/mymuduo so库到/usr/local/lib
#环境变量不用设置
if [ ! -d /usr/local/include/mymuduo ];then
	mkdir /usr/local/include/mymuduo
fi

for header in `ls *.h`
do
	cp $header /usr/local/include/mymuduo
done

cp `pwd`/lib/libmymuduo.so /usr/local/lib

ldconfig



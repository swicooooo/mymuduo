#!/bin/bash

set -e  # 设置日志

# 创建build目录
if [ ! -d `pwd`/build ]; then
    mkdir `pwd`/build
fi

rm -rf `pwd`/build/*

# cmake构建
cd `pwd`/build && cmake .. && make -j8

cd ..

# 拷贝头文件和库文件
if [ ! -d /usr/include/mymuduo ]; then
    mkdir /usr/include/mymuduo
fi

for header in `ls *.h`
do
    cp $header /usr/include/mymuduo/
done

cp `pwd`/lib/libmymuduo.so /usr/lib

ldconfig    # 刷新/usr/bin
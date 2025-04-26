#!/bin/bash

# 定义基础路径
BASE_PATH="/storage/emulated/0/Download/XQ三合一"
echo "正在创建必要目录😛"
# 创建主目录
mkdir -p "$BASE_PATH"

# 创建子目录
mkdir -p "$BASE_PATH/pak"
mkdir -p "$BASE_PATH/打包"
mkdir -p "$BASE_PATH/解包"
mkdir -p "$BASE_PATH/配置"
mkdir -p "$BASE_PATH/提取配置"
mkdir -p "$BASE_PATH/特征码"

# 创建打包和解包的子目录
mkdir -p "$BASE_PATH/打包/dat"
mkdir -p "$BASE_PATH/打包/uexp"
mkdir -p "$BASE_PATH/解包/dat"
mkdir -p "$BASE_PATH/解包/uexp"

# 创建提取配置的子目录和文件
mkdir -p "$BASE_PATH/提取配置/原版"
touch "$BASE_PATH/提取配置/提取结果.py"

# 创建配置文件
touch "$BASE_PATH/配置结果.txt"
touch "$BASE_PATH/配料表.txt"

# 创建特征码文件
touch "$BASE_PATH/特征码/非块美化特征码.txt"
touch "$BASE_PATH/特征码/伪实体特征码.txt"
touch "$BASE_PATH/特征码/提取非块美化特征码.txt"

# 设置权限（可选，根据需要调整）
chmod -R 755 "$BASE_PATH"

sleep 3s
echo "文件夹和文件结构已创建完成"
echo "路径: $BASE_PATH"
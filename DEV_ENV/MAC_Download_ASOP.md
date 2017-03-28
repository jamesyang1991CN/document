
# ASOP 环境搭建 (OSX)


访问 [Android官方网站][1] 可以获得搭建和获取源码的过程介绍，但是需要翻墙，所以只使用repo 国内镜像的方法（osx系统）

## 1. 在mac上建大小写敏感的分区
- 命令：hdiutil create -type SPARSE -fs 'Case-sensitive Journaled HFS+' -size 140g ~/android.dmg
生成android.dmg.sparseimage磁盘驱动
- 如果想改分区的大小，在unmount后修改，命令：
hdiutil resize -size <？？>g ~/android.dmg.sparseimage

## 2.  将分区挂载
- 打开 .bash_profile ，命令：open -e ~/.bash_profile
- 添加挂载和卸载分区的函数
``` python
# mount the android file image
function mountAndroid { hdiutil attach ~/android.dmg.sparseimage -mountpoint /Volumes/android; }
# unmount the android file image
function umountAndroid() { hdiutil detach /Volumes/android; }
```
执行命令：source .bash_profile 更新环境变量

## 3.  安装Xcode Command line
## 4.  安装MacPorts
- 访问[MacPorts][2]网站，下载、安装
- 执行命令：export PATH=/opt/local/bin:$PATH
保证在.bash_profile中，路径：/opt/local/bin在usr/bin之前
##  5.  安装make git 等相关包
- 命令：POSIXLY_CORRECT=1 sudo port install gmake libsdl Git gnupg
##  6.  提升编译速度
- 在.bash_profile中添加如下脚本，提升编译速度
``` python
# set the number of open files to be 1024
ulimit -S -n 1024
```
执行命令：source .bash_profile 更新环境变量

##  7.  安装repo
- mkdir ~/bin
- PATH=~/bin:$PATH
- curl https://storage-googleapis.lug.ustc.edu.cn/git-repo-downloads/repo > ~/bin/repo
- chmod a+x ~/bin/repo
命令执行完 在～／bin／生成repo 并且有执行权
限

##  8.  下载和编译代码
- 第二步中已经将挂载分区的方法放入.bash_profile中，执行命令：mountAndroid
- cd ~/Volumes/android/
- mkdir AOSP
- cd ASOP
- 修改repo文件 将REPO_URL 路径 修改为REPO_URL = 'https://mirrors.tuna.tsinghua.edu.cn/git/git-repo'
- 在～目录下创建文件downlaod.sh
执行：touch download.sh ;open -e download.sh
``` python
#!/bin/bash
PATH=~/bin:$PATH
repo init -u
# Android N code from tsinghua image
https://aosp.tuna.tsinghua.edu.cn/platform/manifest -b android-7.1.1_r9
repo sync -j8
while [ $? = 1 ]; do
    echo "================sync failed, re-sync again ====="
    sleep 3
    repo sync -j8 -f --force-sync
done
```
- 同步代码 执行 . ～／download.sh
- 同步完毕 执行 source build/envsetup.sh；choosecombo 1 aosp_arm 3;make -j8 进行编译

# 问题集锦
在编译时可能遇到的问题：
## 1. Xcode 版本太高导致不支持
*build/core/combo/mac_version.mk:38: *****************************************************
build/core/combo/mac_version.mk:39: * Can not find SDK 10.6 at /Developer/SDKs/MacOSX10.6.sdk
build/core/combo/mac_version.mk:40: *****************************************************
build/core/combo/mac_version.mk:41: *** Stop..  Stop.*

解决方案：http://blog.csdn.net/goyoung/article/details/48682163
## 2. 编译遇到OOM
解决方案：http://blog.csdn.net/u013005791/article/details/52212584







作者：贱贱的杨
从此你们的路上不会孤单，还有贱贱的我



---------

[1]: http://source.android.com/source/downloading.html
[2]: http://www.macports.org/install.php

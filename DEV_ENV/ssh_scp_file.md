# mac osx 软连接问题
- sublime 创建软连接
ln -s /Applications/Sublime\ Text.app/Contents/SharedSupport/bin/subl /usr/local/bin/sublime
- 取消软连接
rm -rf subl 

# 拷贝文件
- ssh 连接到电脑 （目前我的另外一台电脑ip是172.16.17.24 用户是sdduser）
ssh sdduser@172.16.17.24 
输入密码 

- 拷贝文件
scp remote 文件 本地路径

或者
scp -r sdduser@172.16.17.24:远端文件 本地文件 

以上方法可能会出现失败，不拷贝，因此有一种异步方法拷贝，详见：
例：rsync -P --rsh=ssh -r sdduser@172.16.20.218:/local/kaios/ /Volumes/kaios/
# kaios 系统介绍

kaios 系统是属于[FireFox OS][1]平台的系统，和android 系统对比，主要的差距

- android 主要采用java 语言是打造界面和框架， 而kaios 使用gecko（浏览器引擎）使用web技术来实现系统显示和操作
- android 如果厂商定制 一般系统 大于2G，而kaios 一般500M，所以需要的硬件成本更低，适合在功能机上搭载（目前就是如此）如果未来kaios像android 或者OSX 流行起来，加上的大数据或者云服务辅助，kaios搭载在智能机上面，也实现丰富多彩的功能，并且成本会更低（只是未来的设想）
- 缺点很明显，功能不完善，不够流行，反应速度表现一般，但是在一些不发达地区，非洲、印度还是有市场，便宜的价格就能体验VoLte VoWiFi ViWiFi ViLTE 等网络服务


# kaios 结构

![kaios structure](https://github.com/jamesyang1991CN/document/blob/master/picture/494px-FirefoxOS_arch.png)

- 还是依赖linux 系统
- 主要由四层 Application layer\Open Web Plantform Interfaces\infrastructure(基础结构)\Device's Operation System 构成
	- Application layer:
		- 应用层 app层面，采用HTML5／js／CSS 编写应用
		- 包含 js 库 、 Gaia 等 （gaia是应用层需要开发者开发app，gaia 完全采用html5／js／css 调用 开源webAPI，gaia 自带已经实现的lockscreen homescreen 等基本系统应用）
	- Open Web Plantform Interfaces:
		- WebAPI 接口层 主要由 HTML5 APIs、contacts、settings、Webtelephony WebSMS/MMS、NFC bluetooth、system xhr、alarm system messages、camera media web rtc、sensor geolock battery vibration、WebAPIs、connections/uicc、web activities等 webAPIs 组成
		- 属于kaios runtime 、提供webAPI接口 在kaios中被称为Gecko （浏览器引擎）
	- infrastructure(基础结构)
		- 被称为Gonk 包括linux 内核 用户空间HAL 、包含用户空间的库 linux 、libusb、bluz、 其他的与android一样 GPS camera 等 为gecko的提供接口
		- 开源库 rild input\touch opengles、wifi support、Audio Video...
	- Device's Operation System 
		- 包含linux kernel OEM 驱动 等

- 目前ril层是我要研究的方向  在kaios 中不是特别成熟

# debug kaios

## 准备工具
- ubuntu 配置 firfox and adb 配置
- 安装 adb 
- lsusb 查看手机ID 类似 1782:5d04 前面的是厂商编号 后面是产品编号 （拔 插手机，看出手机的ID）
	- cd /etc/udev/rules.d/
	- touch 51-android.rules
	- 编辑内容 SUBSYSTEM=="usb",ATTRS(idVendor)=="1782",ATTRS(idProduct)=="5d04",MODE="0666"
		- SUBSYSTEM 是USB设备
		- ATTRS(idVendor)描述厂商的id号
		- ATTRS（idProduct）描述产品号
		- MODE 是访问权限
- 给文件设置权限
	chmod a+r /etc/udev/rules.d/51-android.rules
- 重启udev
	/etc/init.d/udev restart
- 增加adb_usb.ini写入厂商的id 0x1782
- 重启adb adb kill-server adb start-server adb devices


## webIDE
- firefox 浏览器自带debug 工具类似DDMS developer - webIDE 
- 选中需要debug的应用 ，选中debug的log信息一般全选，然后操作会打印一些操作app的log信息











作者：贱贱的杨
从此你们的路上不会孤单，还有贱贱的我



---------

[1]: http://tech.mozilla.com.tw/

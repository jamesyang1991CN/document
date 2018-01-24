# pjsip workshop
## pjsip 介绍

> 背景：
> PJSIP 由英国Teluu团队主导开发，由Benny Prijono (本尼 普里约诺) 创建，他的名字缩写pj，所以命名PJSIP
 

> 优点：

> 可移植性强：可运行在windows、windowsmobile、linux、unix、MacOS、RTEMS、Symbian

> 内存需求小：编译后只需要150k内存空间

> 支持多种SIP功能以及扩展功能：支持多人会话、事件驱动框架、会话控制（presence）、即时信息、电话传输

> 文档介绍：官网有教程可以学习

> 缺点：

> Demo代码之间关联比较紧密，在编译的时候，需要花费时间寻找依赖关系

> 文档特别多，也容易理不清


- 免费的开源多媒体通信库，实现了基于标准的协议 
- 主要实现的协议 SIP，SDP，RTP，STUN，TURN和ICE 
- 信令协议（SIP）与多媒体框架和NAT穿越功能集成到高级多媒体通信API中

- STUN：（Simple Traversal of UDP over NATs，NAT 的UDP简单穿越）是一种网络协议
- TURN：(使用中继穿越NAT)的协议，它允许一台主机使用中继服务与对端进行报文传输
- ICE：Interactive Connectivity Establishment,即交互式连接建立

## 下载编译 PJSIP windows8 平台

- 平台 windows VS2015
- [pjsip下载地址][1]下载最近压缩包，解压 
- [官方介绍文档][2] 介绍需要依赖的文件配置

我下载的是最新版本 pjproject-2.7.1

## pjproject-2.7.1 目录介绍
```
lib: [PJPROJECT的lib库]
pjlib:[基础框架库]
pjlib-util:[辅助工具库]
pjmedia:[开源的媒体栈]
pjnath:[开源的NAT-T辅助库]
pjsip:[开源的SIP协议栈]
pjsip-apps[demo]

```


## 编译 PJSIP 
### 编译前准备
- 安装VS2015 官网其他版本可能会有问题
- 安装SDK： DirectX SDK、Platform SDK
- 安装视频支持SDK：DirectShow SDK，包含在Windows SDK、SDL、libyuv、OpenH264
- 创建 pjlib/include/pj/config_site.h，配置config_site.h 
```c
#include <pj/config_site_sample.h> //配置了参数
/*支持视频的参数*/
#define PJMEDIA_HAS_VIDEO 1  
#define PJMEDIA_HAS_OPENH264_CODEC 1
#define PJMEDIA_HAS_LIBYUV 1
#define PJMEDIA_VIDEO_DEV_HAS_SDL 1
#define PJMEDIA_VIDEO_DEV_HAS_DSHOW 1

/*ffmpeg 支持*/
#define PJMEDIA_HAS_FFMPEG 1 

```


### 编译
- 打开 pjproject-2.7.1 项目中的 pjproject-vs14.sln

![pjsua](http://img.blog.csdn.net/20180119154706129?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvZW5naW5lZXJfamFtZXM=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)

- 设置pjsua 设置为启动项目 设置为debug 和 win32 平台
- 编译：可能会报错，添加依赖库
- 编译成功后，在pjsip-apps/bin 生成pjsua引用 lib库生成在lib目录下面
- 按照同样的方法 编译sample-debug 可以用来调试案例

### 编译自己的应用
- 引入这些库 pjlib、pjlib-UTIL、pjnath、pjmedia、PJSIP、lib
- 使用头文件 (官方介绍的需要引入的头文件)
```c
         #include <pjlib.h>
         #include <pjlib-util.h>
         #include <pjnath.h>
         #include <pjsip.h>
         #include <pjsip_ua.h>
         #include <pjsip_simple.h>
         #include <pjsua-lib / pjsua.h>
         #include <pjmedia.h>
         #include <pjmedia-codec.h>
```

- 在项目设置中声明PJ_WIN32 = 1宏
- 链接系统特定的库，如：wsock32.lib，ws2_32.lib，ole32.lib，dsound.lib
- 使用视频的库 [Video_Users_Guide][3] (我自己研究的demo 没用，暂时没过关注)
- 然后使用自己的demo

- 客户端 & 服务器端 代码 demo
- 最后进行编译 （可能会出错，慢慢调试）

### 在其他平台的使用
在Android平台，会编译成为一个so库使用，需要重新配置环境，使用头文件 pjsua2 API 和pjsua API
官方推荐使用最新的API接口 pjsua2 API

可以 不使用pjsua2 没有集成的接口，只使用上面提到的基础库




 
















---------


  [1]: http://www.pjsip.org/download.htm
  [2]: http://www.pjsip.org/docs/book-latest/html/index.html
  [3]: https://trac.pjsip.org/repos/wiki/Video_Users_Guide
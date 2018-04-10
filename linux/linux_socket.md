# linux

最近一直再看[gstreamer][1],使用了很多插件和库，虽然我总结了一部分rtsp流程，对于这么大的框架，明显感觉分析不了，比如linux API、Gobject（Glib库）推荐一本入门书籍《linux下c编程》，有案例可以debug


# socket

socket 是网络编程一个最基本的函数，本地应用，远端应用都可以用来通信，基于 tcp 和 udp 传输

![socket](https://img-blog.csdn.net/2018041015113319?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2VuZ2luZWVyX2phbWVz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

函数原型：
```c
int socket(int domain, int type, int protocol);

```
domain 协议族  socket 的地址类型






---
[1]:https://gstreamer.freedesktop.org/

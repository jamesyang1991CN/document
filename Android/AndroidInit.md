# Android Init
## 1. BIOS (CMOS) 
- 开机 手机通电 产生一个复位时序 cpu是第一个被复位的时序 CPU开始执行第一条指令 指令固定在内存地址上 引导程序

## 2. 启动linux

### 2.1 bootloader 
- 将引导程序加载到内存

### 2.2 kernel
- linux 内核加载到 内存

## 3. android 系统启动 （入口 init.rc ）
- 启动init 进程 是用户态所有进程的祖先  内核态叫做内核线程
- 作用：子进程终止处理、应用程序访问设备节点文件、提供属性文件保存系统环境需要的环境变量
- 注册一些消息处理器
- 创建和挂载所需要的文件目录（socket 文件和 虚拟内存文件），然后在DEV文件下生成设备节点文件 然后将标准输入输出 错误输出重定向到这里
- init进程生成输出设备文件 开始解析 init.rc 脚本文件生成服务列表和动作列表 将列表注册到service_list 和 action_list 在init进程中生成全局结构体 调用device_init函数 生成静态设备节点文件
- 全局属性值生成在init 进程中properityinit函数中初始化	 在共享内存区域 创建和初始化属性值 对全局属性的修改 只有init进程可以修改 其他进程要修改，init进程通过后，过程中创建一个socket，执行到显示android logo显示
- 处理事件循环监听事件注册在POLL的文件描述符在poll函数中等待 事件发生 从poll 函数中跳出并处理事件，各种文件描述符都会前来注册
- init.rc 文件分析函数通过read_file函数 parse_conifg函数 用来分析读入的字符串 AIL android Init language
- init.rc 大致分为两个部分 一部分以on关键字开头的动作列表 一部分以service开头的服务列表 
	- 动作列表：主要设置环境变量 生成系统运行所需要的文件和目录 修改相应的权限 并挂载和系统运行相关的目录
	- 挂载文件 主要挂载/system /data 两个目录 android根系统文件准备好 根文件系统大致分为shell使用程序 system目录（提供库和基础应用） data目录（保存用户数据和应用） android 采用闪存设备 其采用yaffs2文件系统 启动的时候挂载到 ／system /data目录 
	- onboot 该部分设置应用的终止条件 、应用程序驱动目录、文件权限，为应用定制OOM条件 OOM来监视内核分配给应用程序的内存
- 服务列表 init.rc 脚本文件 service段落记录init进程启动的进程 init进程启动个的子进程 一次性程序系统相关的Daemon进程
- 应用程序通过驱动程序访问硬件设备，驱动节点文件设备驱动的逻辑文件，应用程序通过设备节点文件访问驱动程序
- 创建设备节点文件的两种方式
	1.根据预先设定的设备文件信息，创建设备节点文件 2.在系统运行中，当设备插入的时候，init进程会接受这一事件，为插入动态创建设备节点文件
- 设备插入，内核会加载相应的驱动程序，驱动程序会调用启动函数probe 将次设备类型保存到/sys文件系统中 然后发送uevent 传递给守护进程 。vevent 是内核向用户控件进程传递信息的信号系统。内核通过uevent 将信息传递到用户空间 守护进程会根据uevent读取设备信息 创建设备节点文件

- linux中所有设备都是抽象成一个文件，内核通过文件描述符表示已被打开的文件 ，文件表述符 0标注输入 1标注输出 2错误处理 设备文件socket 都会获得文件描述符来处理

## 4.fork() 创建子进程 
### 4.1 daemon进程即守护进程 USB守护进程 Debug进程 无线通信连接守护进程 等

### 4.2 fork出ContextManager android 系统服务都要向其注册 然后其他进程才可以调用这个服务

### 4.3 Media Server 这个读物是本地服务不是java 服务	不再通过dalvik装在执行	本地服务单独开启一个进程 包括 Audio flinger 和 camera service

### 4.4 Zygote 所有应用的祖先 用来缩短应用的加载时间 
- 所有应用开启的基石 zygote java代码，需要装载到dalvik VM 执行，所以必须dalvik VM先执行 第一步Dalvik VM 启动 第二步 启动zygote
- 首先生成AppRuntime对象 继承AndroidRuntime 初始化运行Dalvik虚拟机 为运行android应用做好准备 接受main函数传进来的参数 然后初始化虚拟机
- 虚拟机初始化 运行ZygoteInit 类，程序的执行转向虚拟机java代码的执行 首先执行main函数 接下来就是zygote作用

- Zygote 执行后 首先绑定一个套接字 用来做ActivityManager 来的应用请求
- 将应用程序框架中的类，平台资源（图像 xml 字符串）预先加载到内存中 借此提升程序的执行速度 Android也是通过在Zygote创建的时候加载资源 生成信息链以后再有启动应用 fork出子进程和父进程共享信息 也共享虚拟机 （虚拟机创建和初始化的时候很耗时）
- 启动应用之前 启动这些Systemserver 启动Server Thread执行android framework的服务 启动应用的时候都需要JNI向服务管理器Context Manager注册 ，zygote 轮询监听Socket 当有请求到达时读取请求fork出子进程加载进程所需要的类，然后执行程序的main函数 转给Dalvik VM，应用程序起来了 ，关闭套接字 删除请求描述符 防止出现重复启动

#### 4.4.1 System Server android系统中的核心进程 提供了管理应用程序的生命周期 地理位置等 将自身注册到Context Manger 服务管理是基于C语言 需要JNI调用接口 此时Activity Manager service 会启动home应用 

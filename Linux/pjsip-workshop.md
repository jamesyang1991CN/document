# pjsip 

## PJSIP 主要构成
结构图
![pjsua](http://img.blog.csdn.net/20180119154116715?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvZW5naW5lZXJfamFtZXM=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
库的介绍
1. 最基础的库是PJLIB
2. 在基础库的基础上开发出8个不同作用的lib库
3. pjsua API 是可以调用的抽象的接口

![pjsip](http://img.blog.csdn.net/20180119154138122?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvZW5naW5lZXJfamFtZXM=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)
1. 不同的lib库会放在相应库的lib文件夹下面
2. 如果想单独使用库 可以看 每个库的参考手册[Reference Manuals][1]
![这里写图片描述](http://img.blog.csdn.net/20180119154217712?watermark/2/text/aHR0cDovL2Jsb2cuY3Nkbi5uZXQvZW5naW5lZXJfamFtZXM=/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70/gravity/SouthEast)


## PJSUA2

PJSUA2是PJSUA API上面的一个面向对象的抽象。它为构建会话发起协议（SIP）多媒体用户代理应用程序（也就是VoIP / VoIP软电话）提供了高级API。它将信令，媒体和NAT穿越功能整合到易于使用的呼叫控制API，帐户管理，好友列表管理，在线状态和即时消息传递中，以及多媒体功能，如本地会议，文件流，本地播放和语音录制，以及使用STUN，TURN和ICE的强大的NAT穿越技术。

PJSUA2是在PJSUA-LIB API之上实现的。SIP和媒体特性和对象建模遵循PJSUA-LIB提供的（例如，我们仍然有帐号，呼叫，好友等等），但访问它们的API是不同的。

PJSUA2是一个C ++库，您可以pjsip在PJSIP发行版的目录下找到它。本地C ++应用程序可以直接使用C ++库。但PJSUA2不仅仅是一个C ++库。从一开始，它就被设计为可以从高级非本地语言（如Java和Python）访问。这是通过SWIG绑定实现的。感谢SWIG，将来可以相对容易地添加与其他语言的绑定。

PJSUA2 API声明可以pjsip/include/pjsua2在源代码位于中找到pjsip/src/pjsua2。编译PJSIP时会自动生成。

使用pjsua2 C ++ API的好处包括：

1. 更清洁的面向对象的API
2. 用于更高级语言的统一API，例如Java和Python
3. 持久性API
4. 能够在需要时访问PJSUA-LIB和更低级别的库（包括扩展库的能力，例如创建自定义的PJSIP模块，pjmedia_port，pjmedia_transport等）

PJSUA2 C ++ API的一些注意事项是：

1. API不会返回错误，而是使用异常来进行错误报告
2. 它使用标准的C ++库（STL）


如果android 平台编译成so文件，就调用pjsua2 进行跨平台调用




## 学习的案例

### 学习pjsua2 (最简单)
[pjsua2 在线介绍文档][2]


demo

```cpp
/* $Id: pjsua2_demo.cpp 5650 2017-09-18 07:10:11Z nanang $ */

#include <pjsua2.hpp>
#include <iostream>
#include <pj/file_access.h>
#define THIS_FILE 	"pjsua2_demo.cpp"

using namespace pj;
class MyAccount;
class MyCall : public Call{
private:
	MyAccount *myAcc;

public:
	MyCall(Account &acc, int call_id = PJSUA_INVALID_ID): Call(acc, call_id){
		myAcc = (MyAccount *)&acc;
	}
	virtual void onCallState(OnCallStateParam &prm);
};

class MyAccount : public Account{
public:
	std::vector<Call *> calls;
public:
	MyAccount(){}
	~MyAccount(){
		std::cout << "*** Account is being deleted: No of calls="<< calls.size() << std::endl;
	}

	void removeCall(Call *call){
		for (std::vector<Call *>::iterator it = calls.begin();it != calls.end(); ++it){
			if (*it == call) {
				calls.erase(it);
				break;
			}
		}
	}

	virtual void onRegState(OnRegStateParam &prm){
		AccountInfo ai = getInfo();
		std::cout << (ai.regIsActive ? "*** Register: code=" : "*** Unregister: code=")<< prm.code << std::endl;
	}

	virtual void onIncomingCall(OnIncomingCallParam &iprm){
		Call *call = new MyCall(*this, iprm.callId);
		CallInfo ci = call->getInfo();
		CallOpParam prm;
		std::cout << "*** Incoming Call: " << ci.remoteUri << " ["<< ci.stateText << "]" << std::endl;
		calls.push_back(call);
		prm.statusCode = (pjsip_status_code)200;
		call->answer(prm);
	}
};

void MyCall::onCallState(OnCallStateParam &prm){
	PJ_UNUSED_ARG(prm);

	CallInfo ci = getInfo();
	std::cout << "*** Call: " << ci.remoteUri << " [" << ci.stateText<< "]" << std::endl;
	if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
		myAcc->removeCall(this);
		/* Delete the call */
		delete this;
	}
}

static void mainProg4(Endpoint &ep) throw(Error){
	// 3. Init library
	EpConfig ep_cfg;
	ep.libInit(ep_cfg);

	// 4. Create transport
	TransportConfig tcfg;
	tcfg.port = 5060;
	ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
	ep.transportCreate(PJSIP_TRANSPORT_TCP, tcfg);

	// Add account
	AccountConfig acc_cfg;
	acc_cfg.idUri = "sip:localhost";
	MyAccount *acc(new MyAccount);
	acc->create(acc_cfg);

	// 5. Start library
	ep.libStart();
	std::cout << "*** PJSUA2 STARTED ***" << std::endl;

	// Just wait for ENTER key
	std::cout << "Press ENTER to quit..." << std::endl;
	std::cin.get();

	delete acc;
}


int main(){
	int ret = 0;
	/*1. Instantiating the Endpoint*/
	Endpoint ep;

	try {
		/*Creating the Library*/
		ep.libCreate();
		mainProg4(ep);
		ret = PJ_SUCCESS;
	}catch (Error & err) {
		std::cout << "Exception: " << err.info() << std::endl;
		ret = 1;
	}

	try {
		ep.libDestroy();
	}catch (Error &err) {
		std::cout << "Exception: " << err.info() << std::endl;
		ret = 1;
	}

	if (ret == PJ_SUCCESS) {
		std::cout << "Success" << std::endl;
	}else {
		std::cout << "Error Found" << std::endl;
	}

	return ret;
}

```


1. 主要的类 ：Endpoint、Account、Media、Call、Buddy   
2. call 对象 主要的操作有 接听电话，挂断电话，挂断电话，转接电话
3. Buddy 表示远程终端对象 可以订阅远程终端的在线状态，以了解远程终端是否在线/离线等，并且可以向好友发送和接收即时消息
4. 使用这些类的方法：继承这些类对象，重写方法，就可以获得回调的通知事件
5. pjsua2 有自己的工作线程来轮询PJSIP的EVENT
6. pjsip每次处理完事件自动销毁
7. EpConfig（端点配置），AccountConfig（帐户配置）和BuddyConfig（好友配置）继承的PersistentObject类可以从文件加载/保存，是持久化对象




### 每个类的介绍
#### [Endpoint][3]

1. 首先需要实例化一个对象
```c
Endpoint *ep = new Endpoint;
```
通这个对象可以配置参数 包括UA sip参数配置、多媒体参数配置、log参数配置

2. 创建库
```cpp
try {
    ep->libCreate();
} catch(Error& err) {
    cout << "Startup error: " << err.info() << endl;
}
```

3. 然后初始化库和配置参数
```cpp
try {
    EpConfig ep_cfg; //通过ep_cfg 添加需要配置的参数
    // Specify customization of settings in ep_cfg
    ep->libInit(ep_cfg);
} catch(Error& err) {
    cout << "Initialization error: " << err.info() << endl;
}


```
Epconfig 提供的配置

1. UAConfig，指定SIP UA设置。
2. MediaConfig来指定各种媒体全局设置
3. LogConfig 来自定义日志等级


```cpp
EpConfig ep_cfg;
ep_cfg.logConfig.level = 5;
ep_cfg.uaConfig.maxCalls = 4;
ep_cfg.mediaConfig.sndClockRate = 16000; //Sampling rate in Hz


```

```cpp
/**
 * Endpoint configuration
 */
struct EpConfig : public PersistentObject
{
    /** UA config */
    UaConfig        uaConfig;

    /** Logging config */
    LogConfig       logConfig;

    /** Media config */
    MediaConfig     medConfig;

    /**
     * Read this object from a container.
     *
     * @param node      Container to write values from.
     */
    virtual void readObject(const ContainerNode &node) throw(Error);

    /**
     * Write this object to a container.
     *
     * @param node      Container to write values to.
     */
    virtual void writeObject(ContainerNode &node) const throw(Error);

};

```



4. 创建传输，这样就可以 发送和接收sip消息
```cpp
try {
    TransportConfig tcfg;
    tcfg.port = 5060;
    TransportId tid = ep->transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    //TransportId tid = ep->transportCreate(PJSIP_TRANSPORT_TCP, tcfg);
    
} catch(Error& err) {
    cout << "Transport creation error: " << err.info() << endl;
}


```

创建TLS的传输（安全传输）
```cpp
try {
    TransportConfig tcfg;
    tcfg.port = 5061;
    // Optional, set CA/certificate/private key files.
    // tcfg.tlsConfig.CaListFile = "ca.crt";
    // tcfg.tlsConfig.certFile = "cert.crt";
    // tcfg.tlsConfig.privKeyFile = "priv.key";
    // Optional, set ciphers. You can select a certain cipher/rearrange the order of ciphers here.
    // tcfg.ciphers = ep->utilSslGetAvailableCiphers();
    TransportId tid = ep->transportCreate(PJSIP_TRANSPORT_TLS, tcfg);
} catch(Error& err) {
    cout << "Transport creation error: " << err.info() << endl;
}

```

5. 启动库
```cpp
try {
    ep->libStart();
} catch(Error& err) {
    cout << "Startup error: " << err.info() << endl;
}
```

6. 最后就是释放库
```cpp
ep->libDestroy();
delete ep;
```

#### [Account][4]
account 表示用户身份，有SIP消息的URI 作为在From 消息头的内容
account 还绑定AccountConfig 的一些配置 在路由的过程中使用，可以通过订阅account，知道目前的状态，也可以设置主动告诉别人目前状态

1. 使用方法 继承这个类，重写回调方法
```cpp
class MyAccount : public Account
{
public:
    MyAccount() {}
    ~MyAccount() {}

    virtual void onRegState(OnRegStateParam &prm)
    {
        AccountInfo ai = getInfo();
        cout << (ai.regIsActive? "*** Register: code=" : "*** Unregister: code=")
             << prm.code << endl;
    }

    virtual void onIncomingCall(OnIncomingCallParam &iprm)
    {
        Call *call = new MyCall(*this, iprm.callId);

        // Just hangup for now
        CallOpParam op;
        op.statusCode = PJSIP_SC_DECLINE;
        call->hangup(op);

        // And delete the call
        delete call;
    }
};

```
处理的account事件回调，主要有 sip注册状态、来电、状态订阅请求、非好友的即时消息等

这些回调，默认处理 来电不处理、订阅默认接收、忽略非好友的请求

2. 创建无用户账户
是通过ip地址来识别终端、没有特定用户ID，标识为“ sip：192.168.0.15 ”（无用户帐户），而不是“ sip：alice@pjsip.org ”
```cpp
    MyAccount.create()
```
3. 创建账户
配置AccountConfig，然后调用create
```cpp
AccountConfig acc_cfg;
acc_cfg.idUri = "sip:test1@pjsip.org";

MyAccount *acc = new MyAccount;
try {
    acc->create(acc_cfg);
} catch(Error& err) {
    cout << "Account creation error: " << err.info() << endl;
}

```
上面的demo只是提供account 信息，但是并没有注册到sip服务器，如果想注册到sip服务器需要配置服务器地址、还有鉴权信息
```cpp
AccountConfig acc_cfg;
acc_cfg.idUri = "sip:test1@pjsip.org";
acc_cfg.regConfig.registrarUri = "sip:pjsip.org";
acc_cfg.sipConfig.authCreds.push_back( 
    AuthCredInfo("digest"/*scheme*/, "*"/*realm*/, "test1"/*user_name*/, 0/*data_type*/, "secret1"/*data*/) );
                                                 
MyAccount *acc = new MyAccount;
try {
    acc->create(acc_cfg);
} catch(Error& err) {
    cout << "Account creation error: " << err.info() << endl;
}

```


4. AccountConfig 配置介绍
> AccountRegConfig来指定注册设置，例如注册服务器和重试间隔。
> AccountSipConfig来指定SIP设置，例如凭证信息和代理服务器。
> AccountCallConfig来指定呼叫设置，例如是否需要可靠的临时响应（SIP 100rel）。
> AccountPresConfig来指定存在设置，例如是否启用存在发布（PUBLISH）。
> AccountMwiConfig，指定MWI（消息等待指示）设置。
> AccountNatConfig来指定NAT设置，例如是否使用STUN或ICE。
> AccountMediaConfig来指定媒体设置，例如安全RTP（SRTP）相关设置。
> AccountVideoConfig来指定视频设置，例如默认捕获和渲染设备。

```cpp
/**
 * Account configuration.
 */
struct AccountConfig : public PersistentObject
{
    /**
     * Account priority, which is used to control the order of matching
     * incoming/outgoing requests. The higher the number means the higher
     * the priority is, and the account will be matched first.
     */
    int         priority;

    /**
     * The Address of Record or AOR, that is full SIP URL that identifies the
     * account. The value can take name address or URL format, and will look
     * something like "sip:account@serviceprovider".
     *
     * This field is mandatory.
     */
    string      idUri;

    /**
     * Registration settings.
     */
    AccountRegConfig    regConfig;

    /**
     * SIP settings.
     */
    AccountSipConfig    sipConfig;

    /**
     * Call settings.
     */
    AccountCallConfig   callConfig;

    /**
     * Presence settings.
     */
    AccountPresConfig   presConfig;

    /**
     * MWI (Message Waiting Indication) settings.
     */
    AccountMwiConfig    mwiConfig;

    /**
     * NAT settings.
     */
    AccountNatConfig    natConfig;

    /**
     * Media settings (applicable for both audio and video).
     */
    AccountMediaConfig  mediaConfig;

    /**
     * Video settings.
     */
    AccountVideoConfig  videoConfig;

    /**
     * IP Change settings.
     */
    AccountIpChangeConfig ipChangeConfig;

public:
    /**
     * Default constructor will initialize with default values.
     */
    AccountConfig();

    /**
     * This will return a temporary pjsua_acc_config instance, which contents
     * are only valid as long as this AccountConfig structure remains valid
     * AND no modifications are done to it AND no further toPj() function call
     * is made. Any call to toPj() function will invalidate the content of
     * temporary pjsua_acc_config that was returned by the previous call.
     */
    void toPj(pjsua_acc_config &cfg) const;

    /**
     * Initialize from pjsip.
     */
    void fromPj(const pjsua_acc_config &prm, const pjsua_media_config *mcfg);

    /**
     * Read this object from a container node.
     *
     * @param node      Container to read values from.
     */
    virtual void readObject(const ContainerNode &node) throw(Error);

    /**
     * Write this object to a container node.
     *
     * @param node      Container to write values to.
     */
    virtual void writeObject(ContainerNode &node) const throw(Error);
};


```



5. Account 操作
> 管理注册
> 管理好友/联系人
> 管理在线状态



#### [Media][5]
AudioMedia 是一个重要的子类，音频媒体：
 a. 捕获设备的AudioMedia，从声音设备捕获音频。
 b. 播放设备的AudioMedia，播放音频到声音设备。
 c. 呼叫的AudioMedia，以发送和接收来自远程人员的音频。
 d. AudioMediaPlayer播放WAV文件。
 e. AudioMediaRecorder，将音频录制到WAV文件。

1. The Audio Conference Bridge 语音会议沟通
音频媒体源可以使用API​​ AudioMedia.startTransmit（）/ AudioMedia.stopTransmit（）来启动/停止传输到目的地

2. 播放WAV文件
```cpp
AudioMediaPlayer player;
AudioMedia& play_med = Endpoint::instance().audDevManager().getPlaybackDevMedia();
try {
    player.createPlayer("file.wav");
    player.startTransmit(play_med);
} catch(Error& err) {
}
```
默认情况循环播放WAV文件，禁用循环播放使用PJMEDIA_FILE_NO_LOOP，播放完后，就会静音
```cpp
player.createPlayer("file.wav", PJMEDIA_FILE_NO_LOOP);
```

停止播放
```cpp
try {
    player.stopTransmit(play_med);
} catch(Error& err) {
}
```

在指定位置播放
```cpp
player.setPos()
```

3. 录制WAV文件
```cpp
AudioMediaRecorder recorder;
AudioMedia& cap_med = Endpoint::instance().audDevManager().getCaptureDevMedia();
try {
    recorder.createRecorder("file.wav");
    cap_med.startTransmit(recorder);
} catch(Error& err) {
}
```
暂停录音
```cpp
try {
   cap_med.stopTransmit(recorder);
} catch(Error& err) {
}
```
播放录制的文件，需要关闭录音对象
```cpp
delete recorder;
```

4. Local Audio Loopback 本地声音回放 用来测试本地音频设备是否有效
```cpp
cap_med.startTransmit(play_med);
```

5. 正常打电话,电话可以语音和视频，程序可以通过Call.getMedia()检索媒体
```cpp
CallInfo ci = call.getInfo();
AudioMedia *aud_med = NULL;

// Find out which media index is the audio
for (unsigned i=0; i<ci.media.size(); ++i) {
    if (ci.media[i].type == PJMEDIA_TYPE_AUDIO) {
        aud_med = (AudioMedia *)call.getMedia(i);
        break;
    }
}

if (aud_med) {
    // This will connect the sound device/mic to the call audio media
    cap_med.startTransmit(*aud_med);

    // And this will connect the call audio media to the sound device/speaker
    aud_med->startTransmit(play_med);
}
```

6. 第二通电话
```cpp
AudioMedia *aud_med2 = (AudioMedia *)call2.getMedia(aud_idx);
if (aud_med2) {
    cap_med->startTransmit(*aud_med2);
    aud_med2->startTransmit(play_med);
}
```
可以同时和他们说话，但是对方两个人不能通话

7. 电话会议
```cpp
aud_med->startTransmit(*aud_med2);
aud_med2->startTransmit(*aud_med);
```

8. 录制会议

```cpp
cap_med.startTransmit(recorder);
aud_med->startTransmit(recorder);
aud_med2->startTransmit(recorder);
```

#### [Calls][6]

1. call实例化 使用子类
```cpp
class MyCall : public Call
{
public:
    MyCall(Account &acc, int call_id = PJSUA_INVALID_ID)
    : Call(acc, call_id)
    { }

    ~MyCall()
    { }

    // Notification when call's state has changed.
    virtual void onCallState(OnCallStateParam &prm);

    // Notification when call's media state has changed.
    virtual void onCallMediaState(OnCallMediaStateParam &prm);
};

```


2. 打电话 使用makeCall

```cpp
Call *call = new MyCall(*acc);
CallOpParam prm(true); // Use default call settings
try {
    call->makeCall(dest_uri, prm);
} catch(Error& err) {
    cout << err.info() << endl;
}

```

3. 处理来电 onIncomingCall 回调方法

```cpp
void MyAccount::onIncomingCall(OnIncomingCallParam &iprm)
{
    Call *call = new MyCall(*this, iprm.callId);
    CallOpParam prm;
    prm.statusCode = PJSIP_SC_OK;
    call->answer(prm);
}
```

4. call属性
call 实例可以通过getInfo() 得到CallInfo 对象，包含state, media state, 还有对方状态信息等

5. Call Disconnection 
只会在onCallState 回调方法中获取 状态
```cpp
void MyCall::onCallState(OnCallStateParam &prm)
{
    CallInfo ci = getInfo();
    if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
        /* Delete the call */
        delete this;
    }
}
```

6.打电话使用audio 在所有可用的media，选择激活一个使用 

```cpp
void MyCall::onCallMediaState(OnCallMediaStateParam &prm)
{
    CallInfo ci = getInfo();
    // Iterate all the call medias
    for (unsigned i = 0; i < ci.media.size(); i++) {
        if (ci.media[i].type==PJMEDIA_TYPE_AUDIO && getMedia(i)) {
            AudioMedia *aud_med = (AudioMedia *)getMedia(i);

            // Connect the call audio media to sound device
            AudDevManager& mgr = Endpoint::instance().audDevManager();
            aud_med->startTransmit(mgr.getPlaybackDevMedia());
            mgr.getCaptureDevMedia().startTransmit(*aud_med);
        }
    }
}

```
7. call操作
 包含 hanging up, on hold, sending re-INVITE等 

8. Instant Messaging(IM) 及时消息
可以使用Call.sendInstantMessage() 在打电话中，在回调方法Call.onInstantMessageStatus 显示传输状态
还可以发送键入指示 Call.sendTypingIndication
收到的来电IM和输入指示会在Call.onInstantMessage 和Call.onTypingIndication 显示


#### [Buddy (Presence)][7]
buddy 代表远程的对象

1. 使用buddy 继承
```cpp
class MyBuddy : public Buddy
{
public:
    MyBuddy() {}
    ~MyBuddy() {}

    virtual void onBuddyState();
};
```

2. 订阅好友的状态
```cpp
BuddyConfig cfg;
cfg.uri = "sip:alice@example.com";
MyBuddy buddy;
try {
    buddy.create(*acc, cfg);
    buddy.subscribePresence(true);
} catch(Error& err) {
}
```
在回调onBuddyState 获取好友的信息
```cpp
void MyBuddy::onBuddyState()
{
    BuddyInfo bi = getInfo();
    cout << "Buddy " << bi.uri << " is " << bi.presStatus.statusText << endl;
}
```
3. 响应在线订阅请求
通过 Account 的onIncomingSubscribe 回调方法 处理在线订阅请求

4. 更改帐户的状态
```cpp
try {
    PresenceStatus ps;
    ps.status = PJSUA_BUDDY_STATUS_ONLINE;
    // Optional, set the activity and some note
    ps.activity = PJRPID_ACTIVITY_BUSY;
    ps.note = "On the phone";
    acc->setOnlineStatus(ps);
} catch(Error& err) {
}

```

5. Instant Messaging(IM)即时消息
Buddy.sendInstantMessage 发送消息，消息的传输状态 会在Account.onInstantMessageStatus回调中报告
Buddy.sendTypingIndication 将打字指示发给远程终端
接收到的不在呼叫范围内的即时消息和打字指示将在回调函数 Account.onInstantMessage
和Account.onTypingIndication中报告
使用Call.sendInstantMessage和Call.sendTypingIndication，在呼叫中发送IM和键入指示




### [FAQ][8]
官网列出了很多常见的疑问


## 只使用lib库进行开发 的demo

### 服务器端
```cpp
#include <stdio.h>
#include <stdlib.h>
#include <pjlib.h>
#include <pjsip.h>
#include <stdlib.h>
#include <pjsip-simple/publish.h>
#include <pjlib-util.h>
#include <pjsip_ua.h>
#include <pjsip/sip_uri.h>
#pragma comment(lib,"ws2_32.lib") 
#pragma comment(lib,"mswsock.lib")

//#define PRINT
#define SERVER
//#define AGENT
#define PJ_WIN32 

static pj_caching_pool cp;
static pjsip_endpoint *my_endpt;


int end;
unsigned count;

static pj_bool_t on_rx_request( pjsip_rx_data *rdata );
//static pj_bool_t on_rx_response( pjsip_rx_data *rdata );

#ifdef SERVER
static pjsip_module mod_presence_s =
{
    NULL, NULL,             /* prev, next.      */
    { "mod-presence-server", 14 },      /* Name.            */
    -1,                 /* Id           */
    PJSIP_MOD_PRIORITY_APPLICATION, /* Priority         */
    NULL,               /* load()           */
    NULL,               /* start()          */
    NULL,               /* stop()           */
    NULL,               /* unload()         */
    &on_rx_request,         /* on_rx_request()      */
    NULL,               /* on_rx_response()     */
    NULL,               /* on_tx_request.       */
    NULL,               /* on_tx_response()     */
    NULL,               /* on_tsx_state()       */
};
#endif


static void my_init_pjlib(void)
{
    pj_status_t status;

    //初始化pjlib
    status = pj_init();
    if(status != PJ_SUCCESS) printf("pj_init() error");
     
    //初始化缓冲池产生器
    pj_caching_pool_init(&cp,&pj_pool_factory_default_policy,0);
}

int main()
{
    pj_pool_t *pool;
    //pjsip_name_addr *name_addr;
    pjsip_sip_uri *sip_uri;
    pjsip_publishc * publish;
    pj_status_t status;

    /********************  1.初始化PJLIB  ********************/
    my_init_pjlib();

    /**************  2.建立内存池用来分配内存 ****************/
    pool = pj_pool_create( &cp.factory,"mypool",4000,4000,NULL);

    /**************  3.Then init PJLIB-UTIL  *****************/
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /****************  4.建立pjsip_endpoint  *****************/
    status = pjsip_endpt_create(&cp.factory,"myendpt",&my_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /****************  5.add UDP transport to the endpoint  *****************/ 
     /*Alternatively, application can use pjsip_udp_transport_attach() to
     * start UDP transport, if it already has an UDP socket (e.g. after it
     * resolves the address with STUN).
     */ 

    pj_sockaddr_in addr1;
    addr1.sin_family = PJ_AF_INET;
    addr1.sin_addr.s_addr = 0;
    addr1.sin_port = pj_htons(5061);

    status = pjsip_udp_transport_start(my_endpt,&addr1,NULL,1,NULL);
    if(status != PJ_SUCCESS){
        printf("Unable to start UDP transport");
    }

    /***************  6.Init transaction layer  ******************/
     /* This will create/initialize transaction hash tables etc.
     */
    //Register the transaction layer module and stateful util module 
    status = pjsip_tsx_layer_init_module(my_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /***************  7.Initialize UA layer module  *************/
     /* This will create/initialize dialog hash tables etc.
     */
    status = pjsip_ua_init_module(my_endpt, NULL );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /*******************  9.Register our new module  **********************/
    status = pjsip_endpt_register_module(my_endpt,&mod_presence_s);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1)


    for (;!end;) {
    //pj_time_val timeout = {0, 10};
    //pjsip_endpt_handle_events2(my_endpt, &timeout,&count);
    
    pjsip_endpt_handle_events2(my_endpt,NULL,&count);
    
    }

    printf("The nember of events have been handled is : %d \n",count);
    
    /*                  pjsip_endpt_handle_events 
    Poll for events. Application must call this function periodically
    to ensure that all events from both transports and timer heap are
    handled in timely manner. This function, like all other endpoint functions,
    is thread safe, and application may have more than one thread concurrently 
    calling this function.
           pj_status_t  pjsip_endpt_handle_events2 ( pjsip_endpoint *  endpt,  
                                                     const pj_time_val *  max_timeout,  
                                                     unsigned *  count   
                                                    ) 

    Handle events with additional info about number of events that have been handled
    */ 

    return 0;

}

    static pj_bool_t on_rx_request( pjsip_rx_data *rdata ){
        char * rdata_info;
        pj_status_t status;
        pjsip_transaction *tsx;
        pjsip_tx_data *tdata;

        rdata_info = pjsip_rx_data_get_info(rdata);
        
        printf("The received transmission data info is %s \n",rdata_info);
        printf("On_rx_repuest is called");

        //Create and initialize transaction
        status = pjsip_tsx_create_uas(&mod_presence_s,rdata,&tsx);

        //Pass in the initial request message
        pjsip_tsx_recv_msg(tsx,rdata);

        //Create response
        status = pjsip_endpt_create_response(my_endpt,rdata,200,&pj_str("OK"),&tdata);

        //Send response with the specified transaction
        pjsip_tsx_send_msg(tsx,tdata);


        end = 1;

        return PJ_TRUE;
    }

```
```
15:23:01.690 os_core_win32. !pjlib 2.7.1 for win32 initialized
15:23:01.690 sip_endpoint.c  Creating endpoint instance...
15:23:01.691          pjlib  select() I/O Queue created (005E1CB0)
15:23:01.691 sip_endpoint.c  Module "mod-msg-print" registered
15:23:01.691 sip_transport.  Transport manager created.
15:23:01.699    udp005EE6B0  SIP UDP transport started, published address is 10.238.100.43:5061
15:23:01.699 sip_endpoint.c  Module "mod-tsx-layer" registered
15:23:01.699 sip_endpoint.c  Module "mod-stateful-util" registered
15:23:01.699 sip_endpoint.c  Module "mod-ua" registered
15:23:01.699 sip_endpoint.c  Module "mod-presence-s" registered
15:23:10.121 sip_endpoint.c  Processing incoming message: Request msg PUBLISH/cseq=32835 (rdata005F126C)
The received transmission data info is Request msg PUBLISH/cseq=32835 (rdata005F126C)
On_rx_repuest is called
15:23:10.122    tsx005F3C0C  ..Transaction created for Request msg PUBLISH/cseq=32835 (rdata005F126C)
15:23:10.122    tsx005F3C0C  .Incoming Request msg PUBLISH/cseq=32835 (rdata005F126C) in state Null
15:23:10.122    tsx005F3C0C  ..State changed from Null to Trying, event=RX_MSG
15:23:10.122       endpoint  .Response msg 200/PUBLISH/cseq=32835 (tdta005F4414) created
15:23:10.122    tsx005F3C0C  .Sending Response msg 200/PUBLISH/cseq=32835 (tdta005F4414) in state Trying
15:23:10.122    tsx005F3C0C  ..State changed from Trying to Completed, event=TX_MSG
The nember of events have been handled is : 1

```


### 客户端

```cpp
#include <pjlib.h>
#include <pjsip.h>
#include <stdlib.h>
#include <pjsip-simple/publish.h>
#include <pjlib-util.h>
#include <pjsip_ua.h>
#include <pjsip/sip_uri.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib") //链接wpcap.lib这个库 
#pragma comment(lib,"mswsock.lib")

static pj_caching_pool cp;
static pjsip_endpoint *my_endpt;


int end;
unsigned count;

//static pj_bool_t on_rx_request( pjsip_rx_data *rdata );
static pj_bool_t on_rx_response( pjsip_rx_data *rdata );

static pjsip_module mod_presenceua =
{
    NULL, NULL,             /* prev, next.      */
    { "mod-presenceua", 14 },       /* Name.            */
    -1,                 /* Id           */
    PJSIP_MOD_PRIORITY_APPLICATION, /* Priority         */
    NULL,               /* load()           */
    NULL,               /* start()          */
    NULL,               /* stop()           */
    NULL,               /* unload()         */
    NULL,           /* on_rx_request()      */
    &on_rx_response,                /* on_rx_response()     */
    NULL,               /* on_tx_request.       */
    NULL,               /* on_tx_response()     */
    NULL,               /* on_tsx_state()       */
};

static void my_init_pjlib(void)
{
    pj_status_t status;

    //初始化pjlib
    status = pj_init();
    if(status != PJ_SUCCESS) printf("pj_init() error");
     
    //线程池初始化
    pj_caching_pool_init(&cp,&pj_pool_factory_default_policy,0);
}


void pjsip_publishc_callback(struct pjsip_publishc_cbparam *param)
{
    printf("Publish callback is called!");

}


int main()
{
    pj_pool_t *pool;
    //pjsip_name_addr *name_addr;
    pjsip_sip_uri *sip_uri;
    pjsip_publishc * publish;
    pj_status_t status;
    /********************  1.初始化PJLIB  ********************/
    my_init_pjlib();

    /**************  2.建立内存池用来分配内存 ****************/
    pool = pj_pool_create( &cp.factory,"mypool",4000,4000,NULL);

    /**************  3.Then init PJLIB-UTIL  *****************/
    status = pjlib_util_init();
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /****************  4.创建pjsip_endpoint  *****************/
    status = pjsip_endpt_create(&cp.factory,"myendpt",&my_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /****************  5.add UDP transport to the endpoint  *****************/ 
     /*Alternatively, application can use pjsip_udp_transport_attach() to
     * start UDP transport, if it already has an UDP socket (e.g. after it
     * resolves the address with STUN).
     */ 
    pj_sockaddr_in addr;
    addr.sin_family = PJ_AF_INET; 
    addr.sin_addr.s_addr = 0;
    addr.sin_port = pj_htons(5060);

    /*
    * Socket address, internet style.

    struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
    };

    */

    status = pjsip_udp_transport_start(my_endpt,&addr,NULL,1,NULL);
    if(status != PJ_SUCCESS){
        printf("Unable to start UDP transport");
    }else {
        printf(" start UDP transport \n");
    }

    /***************  6.Init transaction layer  ******************/
     /* This will create/initialize transaction hash tables etc.
     */
    //Register the transaction layer module and stateful util module 
    status = pjsip_tsx_layer_init_module(my_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /***************  7.Initialize UA layer module  *************/
     /* This will create/initialize dialog hash tables etc.
     */
    status = pjsip_ua_init_module(my_endpt, NULL );
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /***************  8. Init publish session module  ************/
    /* The publish session module initialization takes additional argument,
     * i.e. a structure containing callbacks to be called on specific
     * occurence of events.
     */
    

    /******************  8.(a)Init the callback for PUBLISH session  **********/
    pjsip_publishc_cb *pub_cb;
    pub_cb = &pjsip_publishc_callback;


    /*****************  8.(b)Initialize publish session module  **************/
    status = pjsip_publishc_init_module(my_endpt);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    
    /*******************  9.Register our new module  **********************/
    status = pjsip_endpt_register_module(my_endpt,&mod_presenceua);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /*******************  Register Server module  *************************/

    /*
     * If URL is specified, then make publish immediately.
     */

    char temp[80] = "sip:mod-presenceua@10.238.100.43";

    char dst[80] = "sip:mod-presenceua@10.238.100.43:5061";
    
    pj_str_t local_uri;
    pj_str_t dst_uri;

    pjsip_dialog *dlg;

    pjsip_tx_data *tdata;

    
    local_uri = pj_str(temp);

    dst_uri = pj_str(dst);

    /* Create UAC dialog */
    status = pjsip_dlg_create_uac(pjsip_ua_instance(), 
                       &local_uri,  /* local URI */
                       NULL,        /* local Contact */
                       &dst_uri,    /* remote URI */
                       &dst_uri,    /* remote target */
                       &dlg);       /* dialog */
    if (status != PJ_SUCCESS) {
        printf("Unable to create UAC dialog");
        return 1;
    }


    


    /*******************  10.Create client publication structure  *******************/
    status = pjsip_publishc_create(my_endpt,0,NULL,pub_cb,&publish);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);


    /*******************  11.Initialize client publication structure  ********/
    pj_str_t p_event = {"presence",8};
    status = pjsip_publishc_init(publish,&p_event,&dst_uri,&local_uri,&dst_uri,1000);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /******************  12.Create PUBLISH request  *******************************/
    status = pjsip_publishc_publish(publish,1,&tdata);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    /*********************  12.Send initial PUBLISH request  ***********************/ 
    /* From now on, the PUBLISH session's state will be reported to us
     * via the publish session callbacks.
     */
    status = pjsip_publishc_send(publish,tdata);
    PJ_ASSERT_RETURN(status == PJ_SUCCESS, 1);

    for (;!end;) {
    //pj_time_val timeout = {0, 10};
    //pjsip_endpt_handle_events2(my_endpt, &timeout,&count);
    
    pjsip_endpt_handle_events2(my_endpt,NULL,&count);
    
    }

    printf("The nember of events have been handled is : %d \n",count);
    

    /*                  pjsip_endpt_handle_events 
    Poll for events. Application must call this function periodically
    to ensure that all events from both transports and timer heap are
    handled in timely manner. This function, like all other endpoint functions,
    is thread safe, and application may have more than one thread concurrently 
    calling this function.
           pj_status_t  pjsip_endpt_handle_events2 ( pjsip_endpoint *  endpt,  
                                                     const pj_time_val *  max_timeout,  
                                                     unsigned *  count   
                                                    ) 

    Handle events with additional info about number of events that have been handled
    */

    return 0;

}

    static pj_bool_t on_rx_response( pjsip_rx_data *rdata ){
        char * rdata_info;
        rdata_info = pjsip_rx_data_get_info(rdata);
        printf("The received transmission data info is %s \n",rdata_info);
        printf("On_rx_response is called");
        end = 1;

        return PJ_TRUE;
    }

```
```

15:23:12.066 os_core_win32. !pjlib 2.7.1 for win32 initialized
15:23:12.066 sip_endpoint.c  Creating endpoint instance...
15:23:12.066          pjlib  select() I/O Queue created (0062AED0)
15:23:12.066 sip_endpoint.c  Module "mod-msg-print" registered
15:23:12.066 sip_transport.  Transport manager created.
15:23:12.066    udp0063A4C8  SIP UDP transport started, published address is 10.
238.100.80:5060
 start UDP transport
15:23:12.066 sip_endpoint.c  Module "mod-tsx-layer" registered
15:23:12.066 sip_endpoint.c  Module "mod-stateful-util" registered
15:23:12.066 sip_endpoint.c  Module "mod-ua" registered
15:23:12.081 sip_endpoint.c  Module "mod-presenceua" registered
15:23:12.081    dlg0063EB94  UAC dialog created
15:23:12.081       endpoint  Request msg PUBLISH/cseq=32834 (tdta0063F7A4) created.
15:23:12.081    tsx006407AC  .Transaction created for Request msg PUBLISH/cseq=32835 (tdta0063F7A4)
15:23:12.081    tsx006407AC  Sending Request msg PUBLISH/cseq=32835 (tdta0063F7A4) in state Null
15:23:12.081  sip_resolve.c  .Target '10.238.100.43:5061' type=Unspecified resolved to '10.238.100.43:5061' type=UDP (UDP transport)
15:23:12.081    tsx006407AC  .State changed from Null to Calling, event=TX_MSG
15:23:12.081 sip_endpoint.c  Processing incoming message: Response msg 200/PUBLISH/cseq=32835 (rdata0063AFEC)
15:23:12.081    tsx006407AC  .Incoming Response msg 200/PUBLISH/cseq=32835 (rdata0063AFEC) in state Calling
15:23:12.081    tsx006407AC  ..State changed from Calling to Completed, event=RX_MSG
Publish callback is called!
15:23:17.091    tsx006407AC  Timeout timer event
15:23:17.091    tsx006407AC  .State changed from Completed to Terminated, event=TIMER
15:23:17.106    tsx006407AC  Timeout timer event
15:23:17.106    tsx006407AC  .State changed from Terminated to Destroyed, event=TIMER
15:23:17.106   tdta0063F7A4  ..Destroying txdata Request msg PUBLISH/cseq=32835(tdta0063F7A4)
15:23:17.106    tsx006407AC  Transaction destroyed!

```












作者：贱贱的杨 
从此你们的路上不会孤单，还有贱贱的我


---------

[1]: https://trac.pjsip.org/repos/
[2]: http://www.pjsip.org/docs/book-latest/html/index.html
[3]: http://www.pjsip.org/docs/book-latest/html/endpoint.html
[4]: http://www.pjsip.org/docs/book-latest/html/account.html
[5]: http://www.pjsip.org/docs/book-latest/html/media.html
[6]: http://www.pjsip.org/docs/book-latest/html/call.html
[7]: http://www.pjsip.org/docs/book-latest/html/presence.html
[8]: https://trac.pjsip.org/repos/wiki/FAQ#doc





# cs call (Android 7)
## dialer 进程
- 点击拨号 触发拨号处理
- com.android.dialer.dialpad.DialpadFragment#handleDialButtonPressed

```java
private void handleDialButtonPressed() {
        if (isDigitsEmpty()) { // No number entered.
            handleDialButtonClickWithEmptyDigits();
        } else {
            final String number = mDigits.getText().toString();
     		    ...
            
                final Intent intent = new CallIntentBuilder(number).
                        setCallInitiationType(LogState.INITIATION_DIALPAD)
                        .build();
                DialerUtils.startActivityWithErrorToast(getActivity(), intent);//-->dial number
                hideAndClearDialpad(false);//清除界面的号码
        }
    }
```
- com/android/dialer/util/DialerUtils#startActivityWithErrorToast

```java
    public static void startActivityWithErrorToast(Context context, Intent intent, int msgId) {
        try {
            if ((IntentUtil.CALL_ACTION.equals(intent.getAction())
                            && context instanceof Activity)) {
                ...

                final boolean hasCallPermission = TelecomUtil.placeCall((Activity) context, intent);//--->dial call
                ...
            } 
        } ...
    }
```
- com.android.dialer.util.TelecomUtil#placeCall  //--> telecom 兼容工具类

```java
    public static boolean placeCall(Activity activity, Intent intent) {
        if (hasCallPhonePermission(activity)) {
            TelecomManagerCompat.placeCall(activity, getTelecomManager(activity), intent);
            return true;
        }
        return false;
    }

```

- com.android.contacts.common.compat.telecom.TelecomManagerCompat#placeCall 

```java
    public static void placeCall(@Nullable Activity activity,
            @Nullable TelecomManager telecomManager, @Nullable Intent intent) {
    	...
        if (CompatUtils.isMarshmallowCompatible()) {
            telecomManager.placeCall(intent.getData(), intent.getExtras());//--> place call
            return;
        }
        activity.startActivityForResult(intent, 0);
    }
```

- android.telecom.TelecomManager#placeCall

```java
    @RequiresPermission(android.Manifest.permission.CALL_PHONE)
    public void placeCall(Uri address, Bundle extras) {
        ITelecomService service = getTelecomService();
        if (service != null) {
            ...
            try {
                service.placeCall(address, extras == null ? new Bundle() : extras,
                        mContext.getOpPackageName());//--place call
            } catch (RemoteException e) {
                Log.e(TAG, "Error calling ITelecomService#placeCall", e);
            }
        }
    }

```
## system 进程
- aidl 通过进程间通信  实现 dialer进程 和 system Telecom service 通信
- 由于 TelecomServiceImpl 实例在 Telecomservice 中绑定服务的时候新建 而 Telecomservice在system进程

- com.android.server.telecom.components.TelecomService#onBind

```java
public class TelecomService extends Service implements TelecomSystem.Component {

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(this, "onBind");
        initializeTelecomSystem(this);
        synchronized (getTelecomSystem().getLock()) {
            return getTelecomSystem().getTelecomServiceImpl().getBinder();
            //新建Telecomservice实例
        }
    }

}
```
- packages/services/Telecomm/AndroidManifest.xml

```xml
        <service android:name=".components.TelecomService"
                android:singleUser="true"
                android:process="system"> <!--system 进程-->
            <intent-filter>
                <action android:name="android.telecom.ITelecomService" />
            </intent-filter>
        </service>
```
- 在 TelecomServiceImpl 实现placeCall
- packages/services/Telecomm/src/com/android/server/telecom/TelecomServiceImpl.java

```java
private final ITelecomService.Stub mBinderImpl = new ITelecomService.Stub() {
	...
		
        @Override
        public void placeCall(Uri handle, Bundle extras, String callingPackage) {
            try {
 

                ...

                synchronized (mLock) {
                    ...
                    try {
                        final Intent intent = new Intent(Intent.ACTION_CALL, handle);
                        ...
                        mUserCallIntentProcessorFactory.create(mContext, userHandle)
                                .processIntent(
                                        intent, callingPackage, hasCallAppOp && hasCallPermission);//-->call process intent 
                    }
                 	...
                }
            ...
        }
}

```
- com.android.server.telecom.components.UserCallIntentProcessor#processIntent
```java
	/**
     * Processes intents sent to the activity.
     *
     * @param intent The intent.
     */
    public void processIntent(Intent intent, String callingPackageName,
            boolean canCallNonEmergency) {
        ...

        if (Intent.ACTION_CALL.equals(action) ||
                Intent.ACTION_CALL_PRIVILEGED.equals(action) ||
                Intent.ACTION_CALL_EMERGENCY.equals(action)) {
            processOutgoingCallIntent(intent, callingPackageName, canCallNonEmergency);//--> call 处理
        }
    }
```
- com.android.server.telecom.components.UserCallIntentProcessor#processOutgoingCallIntent

```java
	private void processOutgoingCallIntent(Intent intent, String callingPackageName,
            boolean canCallNonEmergency) {
               ... 

        sendBroadcastToReceiver(intent);// 将重新封装的intent 处理
    }
```
- com.android.server.telecom.components.UserCallIntentProcessor#sendBroadcastToReceiver
```java
    private boolean sendBroadcastToReceiver(Intent intent) {
        intent.putExtra(CallIntentProcessor.KEY_IS_INCOMING_CALL, false);
        intent.setFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        intent.setClass(mContext, PrimaryCallReceiver.class);
        Log.d(this, "Sending broadcast as user to CallReceiver");
        mContext.sendBroadcastAsUser(intent, UserHandle.SYSTEM);
        // 将intent 交由 PrimaryCallReceiver 处理
        return true;
    }


```
- PrimaryCallReceiver 将 intent 交由  CallIntentProcessor 处理
- PrimaryCallReceiver 属于system 进程 CallIntentProcessor 在里面是实例化也在system进程中运行

```xml
		<receiver android:name=".components.PrimaryCallReceiver"
                android:exported="true"
                android:permission="android.permission.MODIFY_PHONE_STATE"
                android:process="system">
        </receiver>
```
- com.android.server.telecom.CallIntentProcessor#processIntent

```java
	public void processIntent(Intent intent) {
        final boolean isUnknownCall = intent.getBooleanExtra(KEY_IS_UNKNOWN_CALL, false);
        Log.i(this, "onReceive - isUnknownCall: %s", isUnknownCall);

        Trace.beginSection("processNewCallCallIntent");
        if (isUnknownCall) {
            processUnknownCallIntent(mCallsManager, intent);// 处理来路不明的call 
            //目前只在处理stk call 的时候有遇见过就是没有intent 没有封装call 类型
        } else {
            processOutgoingCallIntent(mContext, mCallsManager, intent);//处理call
        }
        Trace.endSection();
    }

```
- com.android.server.telecom.CallIntentProcessor#processOutgoingCallIntent
```java
    static void processOutgoingCallIntent(
            Context context,
            CallsManager callsManager,
            Intent intent) {
    	/**
		 * 获取数据
    	 */
        Uri handle = intent.getData();
        String scheme = handle.getScheme();
        String uriString = handle.getSchemeSpecificPart();

        ...
        // incallUI 启动流程 

        // Send to CallsManager to ensure the InCallUI gets kicked off before the broadcast returns
        Call call = callsManager
                .startOutgoingCall(handle, phoneAccountHandle, clientExtras, initiatingUser);

        if (call != null) {
            
            NewOutgoingCallIntentBroadcaster broadcaster = new NewOutgoingCallIntentBroadcaster(
                    context, callsManager, call, intent, callsManager.getPhoneNumberUtilsAdapter(),
                    isPrivilegedDialer);
            final int result = broadcaster.processIntent();// call ril 层 流程
            final boolean success = result == DisconnectCause.NOT_DISCONNECTED;

            if (!success && call != null) {
                disconnectCallAndShowErrorDialog(context, call, result);
            }
        }
    }

```
- com.android.server.telecom.NewOutgoingCallIntentBroadcaster#processIntent
```java
    @VisibleForTesting
    public int processIntent() {
        Log.v(this, "Processing call intent in OutgoingCallIntentBroadcaster.");
        //获取数据
        Intent intent = mIntent;
        String action = intent.getAction();
        final Uri handle = intent.getData();

        ...
        //voicemail 处理
        //action 预处理 号码

        UserHandle targetUser = mCall.getInitiatingUser();
        Log.i(this, "Sending NewOutgoingCallBroadcast for %s to %s", mCall, targetUser);
        broadcastIntent(intent, number, !callImmediately, targetUser);
        return DisconnectCause.NOT_DISCONNECTED;
    }

```

- com.android.server.telecom.NewOutgoingCallIntentBroadcaster#broadcastIntent
```java
    private void broadcastIntent(
            Intent originalCallIntent,
            String number,
            boolean receiverRequired,
            UserHandle targetUser) {
        ...

        mContext.sendOrderedBroadcastAsUser(
                broadcastIntent,
                targetUser,
                android.Manifest.permission.PROCESS_OUTGOING_CALLS,
                AppOpsManager.OP_PROCESS_OUTGOING_CALLS,
                receiverRequired ? new NewOutgoingCallBroadcastIntentReceiver() : null,
                null,  // scheduler
                Activity.RESULT_OK,  // initialCode
                number,  // initialData: initial value for the result data (number to be modified)
                null);  // 
        //广播发送给NewOutgoingCallBroadcastIntentReceiver 处理
    }

    /**
     * Processes the result of the outgoing call broadcast intent, and performs callbacks to
     * the OutgoingCallIntentBroadcasterListener as necessary.
     */
    public class NewOutgoingCallBroadcastIntentReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            try {
                ...
                    mCallsManager.placeOutgoingCall(mCall, resultHandleUri, gatewayInfo,
                            mIntent.getBooleanExtra(
                                    TelecomManager.EXTRA_START_CALL_WITH_SPEAKERPHONE, false),
                            mIntent.getIntExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                                    VideoProfile.STATE_AUDIO_ONLY));
                }
            } finally {
                ...
            }
        }
    }
```
- com.android.server.telecom.CallsManager#placeOutgoingCall
```java
    @VisibleForTesting
    public void placeOutgoingCall(Call call, Uri handle, GatewayInfo gatewayInfo,
            boolean speakerphoneOn, int videoState) {
       ...	//call 对象的先决条件处理

        if (call.getTargetPhoneAccount() != null || call.isEmergencyCall()) {
            // If the account has been set, proceed to place the outgoing call.
            // Otherwise the connection will be initiated when the account is set by the user.
            call.startCreateConnection(mPhoneAccountRegistrar);//call 开始建立链路
        } ...
    }
```
- com.android.server.telecom.Call#startCreateConnection

```java
	void startCreateConnection(PhoneAccountRegistrar phoneAccountRegistrar) {
        ...
        mCreateConnectionProcessor = new CreateConnectionProcessor(this, mRepository, this,
                phoneAccountRegistrar, mContext);
        mCreateConnectionProcessor.process();//链路处理
    }
```

- com.android.server.telecom.CreateConnectionProcessor#process
```java
   @VisibleForTesting
    public void process() {
        ...
        attemptNextPhoneAccount();//开始不断尝试连接链路
    }

```
- com.android.server.telecom.CreateConnectionProcessor#attemptNextPhoneAccount

```java
    private void attemptNextPhoneAccount() {
        Log.v(this, "attemptNextPhoneAccount");
        

        ...

                mService.createConnection(mCall, this);
          ...  
        
    }

```

- com.android.server.telecom.ConnectionServiceWrapper#createConnection

```java
	@VisibleForTesting
    public void createConnection(final Call call, final CreateConnectionResponse response) {
        Log.d(this, "createConnection(%s) via %s.", call, getComponentName());
        BindCallback callback = new BindCallback() {
            @Override
            public void onSuccess() {
                //回调 成功处理
                try {
                    mServiceInterface.createConnection(
                            call.getConnectionManagerPhoneAccount(),
                            callId,
                            new ConnectionRequest(
                                    call.getTargetPhoneAccount(),
                                    call.getHandle(),
                                    extras,
                                    call.getVideoState(),
                                    callId),
                            call.shouldAttachToExistingConnection(),
                            call.isUnknown());
                            //mServiceInterface 是实现和IConnectionServiceIcon实现通信
                } catch (RemoteException e) {
                    ...
                }
            }

            @Override
            public void onFailure() {
                //失败的处理
            }
        };

        mBinder.bind(callback, call);
    }

```

## phone 进程
- 进程间通信 ConnectionService 属于phone进程
- java/android/telecom/ConnectionService.java
```java
	private final IBinder mBinder = new IConnectionService.Stub() {
		@Override
    	public void createConnection(
            PhoneAccountHandle connectionManagerPhoneAccount,
            String id,
            ConnectionRequest request,
            boolean isIncoming,
            boolean isUnknown) {
        ...
        mHandler.obtainMessage(MSG_CREATE_CONNECTION, args).sendToTarget();
        //handler 发送消息 创建链路
    }

```
- com/android/services/telephony/TelephonyConnectionService.java
- TelephonyConnectionService 继承 ConnectionService，phone 进程在开机的时候就启动
- 在handler实例处理case MSG_CREATE_CONNECTION
```java
private final Handler mHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                ...
                case MSG_CREATE_CONNECTION: {
                    SomeArgs args = (SomeArgs) msg.obj;
                    try {
                        ...
                        if (!mAreAccountsInitialized) {
                            Log.d(this, "Enqueueing pre-init request %s", id);
                            mPreInitializationConnectionRequests.add(new Runnable() {
                                @Override
                                public void run() {
                                    createConnection(
                                            connectionManagerPhoneAccount,
                                            id,
                                            request,
                                            isIncoming,
                                            isUnknown);//子线程创建链路
                                }
                            });
                        } ...
                    break;
                }
                ...
            }
        }
    };

```
- android.telecom.ConnectionService#createConnection

```java
    private void createConnection(
            final PhoneAccountHandle callManagerAccount,
            final String callId,
            final ConnectionRequest request,
            boolean isIncoming,
            boolean isUnknown) {
        ...

        Connection connection = isUnknown ? onCreateUnknownConnection(callManagerAccount, request)
                : isIncoming ? onCreateIncomingConnection(callManagerAccount, request)
                : onCreateOutgoingConnection(callManagerAccount, request);//---> MO
        Log.d(this, "createConnection, connection: %s", connection);
        
        ...
    }

```
- TelephonyConnectionService 重写了 onCreateOutgoingConnection
- com.android.services.telephony.TelephonyConnectionService#onCreateOutgoingConnection
```java
    @Override
    public Connection onCreateOutgoingConnection(
            PhoneAccountHandle connectionManagerPhoneAccount,
            final ConnectionRequest request) {
        Log.i(this, "onCreateOutgoingConnection, request: " + request);
...
        
            if(resultConnection instanceof TelephonyConnection) {
                placeOutgoingConnection((TelephonyConnection) resultConnection, phone, request);
            }
            return resultConnection;
    }
    
```

- com.android.internal.telephony.GsmCdmaPhone 继承 phone

```java
    private void placeOutgoingConnection(
            TelephonyConnection connection, Phone phone, int videoState, Bundle extras) {
        String number = connection.getAddress().getSchemeSpecificPart();

        ...
        try {
            if (phone != null) {
                originalConnection = phone.dial(number, null, videoState, extras);
                //----> MO dial to ril
            }
        } catch (CallStateException e) {
            ...
        }
        ...
    }

```
- Phone 对象的来源
- com.android.internal.telephony.PhoneFactory#makeDefaultPhone
- phone进程在初始化的时候，通过网络状态选择何种phone gsm cdma LTE ims
```java
    public static void makeDefaultPhone(Context context) {
        synchronized (sLockProxyPhones) {
            if (!sMadeDefaults) {
                sContext = context;

 				sCommandsInterfaces = new RIL[numPhones];//初始化 mCi
                ...
               
                for (int i = 0; i < numPhones; i++) {
                    sPhones[i].startMonitoringImsService();//ims service
                }

                ...
                //根据网络类型选择  phone

                for (int i = 0; i < numPhones; i++) {
                    Phone phone = null;
                    int phoneType = TelephonyManager.getPhoneType(networkModes[i]);
                    if (phoneType == PhoneConstants.PHONE_TYPE_GSM) {
                        phone = new GsmCdmaPhone(context,
                                sCommandsInterfaces[i], sPhoneNotifier, i,//将 sCommandsInterfaces赋值给 mCi 即RIL
                                PhoneConstants.PHONE_TYPE_GSM,
                                TelephonyComponentFactory.getInstance());
                    } else if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                        phone = new GsmCdmaPhone(context,
                                sCommandsInterfaces[i], sPhoneNotifier, i,//将 sCommandsInterfaces赋值给 mCi 即RIL
                                PhoneConstants.PHONE_TYPE_CDMA_LTE,
                                TelephonyComponentFactory.getInstance());
                    }
                    

                    sPhones[i] = phone;
                }

               ...

                ...
            }
        }
    }

```
- com.android.phone.PhoneGlobals#onCreate
- PhoneGlobals 属于phone 进程初始化 GsmCdmaPhone 对象也属于phone进程
```java
public void onCreate() {
        if (VDBG) Log.v(LOG_TAG, "onCreate()...");
...
        if (mCM == null) {
            // Initialize the telephony framework
            PhoneFactory.makeDefaultPhones(this);
            ...
        }
}
```

- phone.dial 使用的对象是 GsmCdmaPhone#dial
```java
@Override
    public Connection dial(String dialString, UUSInfo uusInfo, int videoState, Bundle intentExtras)
            throws CallStateException {
        ...
        //ims call

        if ((imsUseEnabled && (!isUt || useImsForUt)) || useImsForEmergency) {
            try {
                if (DBG) logd("Trying IMS PS call");
                return imsPhone.dial(dialString, uusInfo, videoState, intentExtras);
            } catch (CallStateException e) {
                ...
            }
        }

       ...
        if (DBG) logd("Trying (non-IMS) CS call");

        if (isPhoneTypeGsm()) {//-->GSM   
            return dialInternal(dialString, null, VideoProfile.STATE_AUDIO_ONLY, intentExtras);
        } else {// --> other call  --->dial
            return dialInternal(dialString, null, videoState, intentExtras);
        }
    }

```
- com.android.internal.telephony.GsmCdmaPhone#dialInternal
```java
@Override
    protected Connection dialInternal(String dialString, UUSInfo uusInfo, int videoState,
                                      Bundle intentExtras)
            throws CallStateException {
            ...

        if (isPhoneTypeGsm()) {
            // handle in-call MMI first if applicable
            ...
        } else {
            return mCT.dial(newDialString);//--->calltracker dial
        }
    }
```
- com.android.internal.telephony.GsmCdmaCallTracker#dial(String dialString)
```java
	public Connection dial(String dialString) throws CallStateException {
        if (isPhoneTypeGsm()) {//GSM网络
            return dial(dialString, CommandsInterface.CLIR_DEFAULT, null);
        } else {//其他网络 --> dial
            return dial(dialString, CommandsInterface.CLIR_DEFAULT);
        }
    }
```
- 假装 通过3g 4G网络拨打的电话
- com.android.internal.telephony.GsmCdmaCallTracker#dial(java.lang.String, int)
```java
private Connection dial(String dialString, int clirMode) throws CallStateException {
       
		...
            // In Ecm mode, if another emergency call is dialed, Ecm mode will not exit.
            if(!isPhoneInEcmMode || (isPhoneInEcmMode && isEmergencyCall)) {
                mCi.dial(mPendingMO.getAddress(), clirMode, obtainCompleteMessage());
                //RIL dial--->
            } 
            ...
        }

        ...

        return mPendingMO;
    }

```

- GsmCdmaPhone 初始化 时 初始化 mCi = sCommandsInterfaces 和 GsmCdmaCallTracker 因此 mCi是RIL对象
```java
	public GsmCdmaCallTracker (GsmCdmaPhone phone) {
        this.mPhone = phone;
        mCi = phone.mCi;  在初始化中看出时RIL
        ...
    }
```
- com.android.internal.telephony.RIL#dial(java.lang.String, int, com.android.internal.telephony.UUSInfo, android.os.Message)
```java

	@Override
    public void
    dial(String address, int clirMode, UUSInfo uusInfo, Message result) {
        RILRequest rr = RILRequest.obtain(RIL_REQUEST_DIAL, result);

        // 封装rr数据

        send(rr);//--->
    }
```
- com.android.internal.telephony.RIL#send
```java
	private void send(RILRequest rr) {
        Message msg;

        ...

        msg = mSender.obtainMessage(EVENT_SEND, rr); //--> send到线程
        acquireWakeLock(rr, FOR_WAKELOCK);
        msg.sendToTarget();
    }

```
- com.android.internal.telephony.RIL
```java
class RILSender extends Handler implements Runnable {
	//***** Handler implementation
        @Override public void
        handleMessage(Message msg) {
            ...

            switch (msg.what) {
                case EVENT_SEND:
	                try {
                        LocalSocket s;

                        s = mSocket; //socket 通信
                        ...
                        s.getOutputStream().write(dataLength);
                        s.getOutputStream().write(data); // --> write to rild
                        
                    } 

	                break;
            }
        }
}
```
- android.net.LocalSocketImpl#writeba_native  
- 调用native 方法 写入数据流中
```java
private native void writeba_native(byte[] b, int off, int len,
            FileDescriptor fd) throws IOException;
```
- frameworks/base/core/jni/android_net_LocalSocketImpl.cpp
- 通过JNI 调用 数据流通过 socket_writeba 写入

```cpp
static const JNINativeMethod gMethods[] = {
     /* name, signature, funcPtr */
    {"connectLocal", "(Ljava/io/FileDescriptor;Ljava/lang/String;I)V",
                                                (void*)socket_connect_local},
    {"bindLocal", "(Ljava/io/FileDescriptor;Ljava/lang/String;I)V", (void*)socket_bind_local},
    {"read_native", "(Ljava/io/FileDescriptor;)I", (void*) socket_read},
    {"readba_native", "([BIILjava/io/FileDescriptor;)I", (void*) socket_readba},
    {"writeba_native", "([BIILjava/io/FileDescriptor;)V", (void*) socket_writeba},
    {"write_native", "(ILjava/io/FileDescriptor;)V", (void*) socket_write},
    {"getPeerCredentials_native",
            "(Ljava/io/FileDescriptor;)Landroid/net/Credentials;",
            (void*) socket_get_peer_credentials}
};

//写入数据
static void socket_writeba (JNIEnv *env, jobject object,
        jbyteArray buffer, jint off, jint len, jobject fileDescriptor)
{
    int fd;
    int err;
    jbyte* byteBuffer;

    ...


    fd = jniGetFDFromFileDescriptor(env, fileDescriptor);

    ...

    byteBuffer = env->GetByteArrayElements(buffer,NULL);//buffer 缓冲区

    ...

    err = socket_write_all(env, object, fd,
            byteBuffer + off, len);
    UNUSED(err);
    // A return of -1 above means an exception is pending

    env->ReleaseByteArrayElements(buffer, byteBuffer, JNI_ABORT);
}
```
- frameworks/base/core/jni/android_net_LocalSocketImpl.cpp
```cpp
/**
 * Writes all the data in the specified buffer to the specified socket.
 *
 * Returns 0 on success or -1 if an exception was thrown.
 */
static int socket_write_all(JNIEnv *env, jobject object, int fd,
        void *buf, size_t len)
{
    ssize_t ret;
    struct msghdr msg;
    unsigned char *buffer = (unsigned char *)buf;
    memset(&msg, 0, sizeof(msg));

    ...
    /**
     * rilc 和rild 通信采用linux 的Unix域socket 进行通信 采用函数socketpair 
     * 进程调用sendmsg 向通道发送消息 由内核处理，将此时打开的	描述符发送给接收方
     * 另一个进程调用recvmsg 从通道接收消息
     * ps: 
     * 1.传递的描述符 在发送进程和接受进程 分别单独的创建，都在内核的文件表中，但是都指向相同的项
     * 2.进程间可以传递的描述符 pipe open mkfifo socket accept 函数返回的描述符
     * 3.描述符 从sendmsg 到 recvmsg 的过程中 标记为“inflight”，即使发送方想关闭描述符，内核将接收进程保持打开状态，发送描述符使其引用计数加一
     * 4.描述符使用结构体cmsghdr的msg_control 发送
	 * struct msghdr {  
     *	void       *msg_name;   msg_name 在调用 recvmsg 时指向接收地址，在调用 sendmsg 时指向目的地址
     *	socklen_t    msg_namelen;  套接口地址长度
     *	struct iovec  *msg_iov;  msg_iov 成员指向一个 struct iovec 数组
     *	size_t       msg_iovlen;  数组长度
     *	void       *msg_control;  指向附属数据缓冲区
     *	size_t       msg_controllen;  附属缓冲区的大小
     *	int          msg_flags;  接收信息的标记位
	 * };   
	 * 套接口地址成员 msg_name msg_namelen
	 * I/O 向量引用  msg_iov msg_iovlen 数据缓冲区
	 * 附属数据缓冲区成员 msg_control 与 msg_controllen ，描述符就是通过它发送的 
	 *
	 * iovc 结构体在 sys/uio.h 
	 * struct iovec {  
     *	ptr_t iov_base;  Starting address 
     *	size_t iov_len;  Length in bytes   
	 * };  
	 * 有了多个缓冲区 的数组readv 和 writev 比read write 因为多个缓冲区的原因 读写效率更高
	 * 
	 * 附属数据
	 *  struct cmsghdr {  
     *		socklen_t cmsg_len;  附属数据的字节计数，这包含结构头的尺寸。这个值是由CMSG_LEN()宏计算的
     *		int       cmsg_level;  初始的协议 级别 例如：SOL_SOCKET
     *		int       cmsg_type;  协议细控制信息类型(例如，SCM_RIGHTS)  附属数据对象是一个文件描述符
     *		\/*u_char     cmsg_data[]; *\/ 不实际存在，指明实际的额外附属数据所在的位置
	 *	};  
	 * CMSG_SPACE()宏计算附属数据及头部所需的空间
	 * 例：
	 * int fd; \/* File Descriptor *\/
	 * char abuf[CMSG_SPACE(sizeof fd)]; 
	 * abuf[]中声明了足够的缓冲区空间来存放头部，填充字节以及附属数据本身
	 * 
	 * CMSG_DATA()宏 接受一个指向cmsghdr结构的指针 
	 * 返回的指针值指向跟随在头部以及填充字节之后的附属数据的第一个字节
	 * 例：
	 * struct cmsgptr *mptr;
	 * int fd; \/* File Descriptor *\/
	 * . . .
	 * fd = *(int *)CMSG_DATA(mptr);指针mptr指向一个描述文件描述符的可用的附属数据信息头部
	 * 
	 * CMSG_FIRSTHDR()宏 返回一个指向附属数据缓冲区内的第一个附属对象的struct cmsghdr指针
	 * msghdr结构的指针 使用msg_control 和 msg_controllen确定缓冲区存在附属对象
     * 如果不存在返回 指向null的指针 存在则指向 存在的第一个cmsghdr 
	 *
	 *
	 *  #include <sys/types.h>  
     *  #include <sys/socket.h>  
	 *	int sendmsg(int s, const struct msghdr *msg, unsigned int flags);  
	 *	int recvmsg(int s, struct msghdr *msg, unsigned int flags);
	 * s， 套接字通道，对于 sendmsg 是发送套接字，对于 recvmsg 则对应于接收套接字；
	 * msg ，信息头结构指针
	 * flags ， 可选的标记位， 这与 send 或是 sendto 函数调用的标记相同
	 * 函数的返回值为实际发送 / 接收的字节数。否则返回 -1 表明发生了错误。
	 * *** 参考 APUE 的高级 I/O 部分**   
     */

    struct cmsghdr *cmsg;//申明附属数据对象指针
    int countFds = outboundFds == NULL ? 0 : env->GetArrayLength(outboundFds);
    int fds[countFds];//数组文件描述符的集合
    char msgbuf[CMSG_SPACE(countFds)];//创建足够多的字符缓冲区

    // Add any pending outbound file descriptors to the message
    if (outboundFds != NULL) {//存在文件描述符

        ...

        for (int i = 0; i < countFds; i++) {
            jobject fdObject = env->GetObjectArrayElement(outboundFds, i);
            ...

            fds[i] = jniGetFDFromFileDescriptor(env, fdObject);//给文件描述符数组赋值
            ...
        }

        // See "man cmsg" really
        msg.msg_control = msgbuf; //缓冲区赋值给msg_control
        msg.msg_controllen = sizeof msgbuf;
        cmsg = CMSG_FIRSTHDR(&msg); //宏 返回一个指向附属数据缓冲区第一个附属对象cmsghdr指针
        cmsg->cmsg_level = SOL_SOCKET;
        cmsg->cmsg_type = SCM_RIGHTS;//文件描述符
        cmsg->cmsg_len = CMSG_LEN(sizeof fds);
        memcpy(CMSG_DATA(cmsg), fds, sizeof fds);//内存拷贝
    }

    // We only write our msg_control during the first write
    while (len > 0) {
        struct iovec iv;
        memset(&iv, 0, sizeof(iv));

        iv.iov_base = buffer;
        iv.iov_len = len;

        msg.msg_iov = &iv;
        msg.msg_iovlen = 1;
        //以上设置 读写缓冲区的大小

        do {
            ret = sendmsg(fd, &msg, MSG_NOSIGNAL);// 表示发送动作不愿被SIGPIPE信号中断
        } while (ret < 0 && errno == EINTR);
        ...
        buffer += ret;
        len -= ret;

        // Wipes out any msg_control too
        memset(&msg, 0, sizeof(msg));
    }

    return 0;
}
```
- 	由于代码时android原生 所以没有modem 部分有所缺失，暂时无法分析，后面继续分析 recvmsg 和kernel部分














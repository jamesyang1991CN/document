# [mozilla][1] API参考网址 
- 手机中的API与网站API的区别，手机中API比mozilla网站API 多了通信模块等其他手机需要的API接口，这部分mozilla已经不提供更新的帮助，只能靠自己设计
# cs call
## dialer 
- path:apps/apps/communications/dialer/
- 入口：apps/apps/communications/manifest.webapp

```javascript
"entry_points": {
    "dialer": {
      "launch_path": "/dialer/index.html#keyboard-view", //dialer 启动入口 keyboard-view是id
      "name": "Phone"
    ...
  },
```

- apps/apps/communications/dialer/js/keyboard_view.js

```javascript

var KeyboardView = {
	/**
	 * 监听keydown 确定call 事件
	 */
	 _initActionList: function kb_initActionMenu() {
    	...
    	var config = {
      		contexts: {
      			...
        		'dialed_number': {
          			items: [{
            			l10nId: 'call',
            			method: function() {
              				self._makeCall(KeypadManager.phoneNumber());//打电话->
            			}
    			...
         			}]
         		},
        };
 		...
      
    },

  	_makeCall: function kb_makeCall(phoneNumber) {
    	if (phoneNumber.length > 0) {
      		LazyLoader.load([
        		'/shared/js/sim_settings_helper.js',
      		], function loaded() {
        		SimSettingsHelper.getCardIndexFrom('outgoingCall', function(defaultCardIndex) {
          			CallHandler.call(phoneNumber, defaultCardIndex);//callhandler 处理call->
        		});
      		});
    	}
    }
};

```

- apps/apps/communications/dialer/js/dialer.js

```javascript
var CallHandler = (function callHandler() {
  /* === Calls === */
  function call(number, cardIndex) {
    var activity = null;
    /**
     *工程模式判断
     */

    /**
     * MMI code 判断
     */
    
    /**
     * 更新号码显示
     */

    /**
     * 出错判断
     */

     /**
      *  使用telephony 接口 拨打电话
      */

    LazyLoader.load(['/shared/js/dialer/telephony_helper.js'], function() {
      TelephonyHelper.call(
        number, cardIndex, oncall, connected, disconnected, error);
    });
  }
})();

```

- gaia/gaia/shared/js/dialer/telephony_helper.js

```javascript
var TelephonyHelper = (function() {
  var call = function t_call(number, cardIndex, oncall, onconnected,
                             ondisconnected, onerror) {
	/*过滤非法数字*/

    var conn = navigator.mozMobileConnections &&
      navigator.mozMobileConnections[cardIndex];//判断语音 数据连接

    if (!conn || !conn.voice) {
      // No voice connection, the call won't make it
      loadTelephonyMessages(function() {
        TelephonyMessages.displayMessage('NoNetwork');//无网络的判断语音和数据连接
      });
      return;
    }

    startDial(cardIndex, conn, sanitizedNumber, oncall, onconnected,
              ondisconnected, onerror);
  };

 

  function startDial(cardIndex, conn, sanitizedNumber, oncall, onconnected,
                     ondisconnected, onerror) {
    ...
    var telephony = navigator.mozTelephony;
    /**
     * 判断SIM卡的状态
     */
    LazyLoader.load('/shared/js/icc_helper.js', function() {
      ...

      /**
       * 判断sim卡unknow 还是 illegal
       */
      
      /**
       * 判断是否仅支持紧急号码
       */

        // If the mobileConnection has a sim card we let gecko take the
        // default service, otherwise we force the first slot.
   		...
        callPromise = telephony.dial(baseNumber, cardIndex);//调用webidl接口 进入gecko
 	});
 }
})();

```
- gecko/gecko/dom/webidl/Telephony.webidl

```javascript
/**
   * Make a phone call or send the mmi code depending on the number provided.
   *
   * TelephonyCall - for call setup
   * MMICall - for MMI code
   */
  [Throws]
  Promise<(TelephonyCall or MMICall)> dial(DOMString number, optional unsigned long serviceId);

  [Throws]
  Promise<TelephonyCall> dialEmergency(DOMString number, optional unsigned long serviceId);

```
- gecko/gecko/dom/telephony/Telephony.cpp

```cpp
// Telephony WebIDL

already_AddRefed<Promise>
Telephony::Dial(const nsAString& aNumber, const Optional<uint32_t>& aServiceId,
                ErrorResult& aRv)
{
  uint32_t serviceId = GetServiceId(aServiceId);
  RefPtr<Promise> promise = DialInternal(serviceId, aNumber, false, aRv);// --->dial
  return promise.forget();
}

already_AddRefed<Promise>
Telephony::DialEmergency(const nsAString& aNumber,
                         const Optional<uint32_t>& aServiceId,
                         ErrorResult& aRv)
{
  uint32_t serviceId = GetServiceId(aServiceId);
  RefPtr<Promise> promise = DialInternal(serviceId, aNumber, true, aRv);
  return promise.forget();
}
}


already_AddRefed<Promise>
Telephony::DialInternal(uint32_t aServiceId, const nsAString& aNumber,
                        bool aEmergency, ErrorResult& aRv)
{
...

  nsCOMPtr<nsITelephonyDialCallback> callback =
    new TelephonyDialCallback(GetOwner(), this, promise);

  nsresult rv = mService->Dial(aServiceId, aNumber, aEmergency, callback);//---> service dial
...

  return promise.forget();
}


// static 初始化时获取service
already_AddRefed<Telephony>
Telephony::Create(nsPIDOMWindowInner* aOwner, ErrorResult& aRv)
{

  nsCOMPtr<nsITelephonyService> ril =
    do_GetService(TELEPHONY_SERVICE_CONTRACTID);
 

  RefPtr<Telephony> telephony = new Telephony(aOwner);

  telephony->mService = ril;
  telephony->mListener = new Listener(telephony);
  telephony->mCallsList = new CallsList(telephony);
  telephony->mGroup = TelephonyCallGroup::Create(telephony);
}
```

- gecko/gecko/dom/telephony/nsITelephonyService.idl

```cpp
 /**
   * Functionality for making and managing phone calls.
   */
  void dial(in unsigned long clientId, in DOMString number,
            in boolean isEmergency, in nsITelephonyDialCallback callback);
```

- GENERATED FROM nsITelephonyService.idl to nsITelephonyService.h 
- out/out/target/product/msm8909_512/obj/objdir-gecko/dist/include/nsITelephonyService.h
- 通过 include 头文件启用vendor 的内容
- vendor/vendor/qcom/proprietary/b2g_telephony/nsTelephonyService.cpp

```javascript
NS_IMETHODIMP nsTelephonyService::Dial(uint32_t clientId,
    const nsAString &number, bool isEmergency,
    nsITelephonyDialCallback *callback) {
  Phone* phone = GetPhoneByClientId(clientId);
  if ((nullptr == callback) || (nullptr == phone)) {
    QLOGW("Dial called with bad parameters");
    return NS_ERROR_INVALID_ARG;
  }
  phone->Dial(number, isEmergency, new TelephonyCbHelper(callback));//--> dial
  return NS_OK;
}
```
- vendor/vendor/qcom/proprietary/b2g_telephony/Phone.cpp

```cpp
void Phone::Dial(const nsAString& dialString, bool isDialEmergency,
    TelephonyCbHelper* cbHelper) {
  // Both CLIR mode and dialing number may get modified when calling ProcessMmi
  // if the dialing string is a temporary mode CLIR request
  int tempClirMode = CLIR_DEFAULT;
  nsString dialingNumber(dialString);

  // If not on CDMA (i.e. when camped on GSM or IMS), then check if the dialing
  // request is an MMI code.
  if (!IsRatTypeCdma() || IsImsInService()) {
    if (mSuplServices->ProcessMmi(dialingNumber, tempClirMode, cbHelper)) {
      // Dial request was handled as a MMI request
      return;
    }
  }

  // Make a normal dial request
  mCallTracker->Dial(dialingNumber, isDialEmergency, tempClirMode,  //-->dial
      ims::CALL_DOMAIN_UNKNOWN, cbHelper);
}

```
- vendor/vendor/qcom/proprietary/b2g_telephony/CallTracker.cpp

```cpp
/**
 * Called by content process to dial a call
 */
void CallTracker::Dial(nsString& dialString, bool isDialEmergency, int clirMode,
  ims::CallDomain domain, TelephonyCbHelper* cbHelper) {
 ...

  if (mPendingMO->mAddress.IsEmpty()) {
    QLOGE("Phone number is invalid");
    mPendingMO->OnDisconnect(DISCONNECT_CAUSE_UNOBTAINABLE_NUMBER);
    mForegroundCall.ClearDisconnected(); // Remove dialing call; go to Idle
    mPendingMO = nullptr; // Release the dialing call
  } else if (!isDialRequestPending) {
    // if isDialRequestPending is true, we postpone the dial
    // request for the second call till we get the hold confirmation
    // for the first call.
    QLOGV("Dialing %s %d", NSSTRING_TO_CSTR(dialString),
        mPendingMO->GetState());
    if ((mState != PHONE_STATE_IDLE) && !mForegroundCall.IsIms() &&
          mPhone->IsRatTypeCdma()) {
      mRil->SendCdmaFlash(this, NSCSTRING_TO_CSTR_NC(mPendingMO->mAddress));
    } else {
      mClirMode = clirMode;
      DialPendingCall(); //--> CT dial
    }
  }

  UpdatePhoneState();
}

/**
 * Dial the call after the result of switch command is
 * received
 */
void CallTracker::DialPendingCall() {
	...
    mRil->Dial(this, NSCSTRING_TO_CSTR_NC(mPendingMO->mAddress), mClirMode);//Ril dial
 ...
  UpdatePhoneState();
}
```

- RIL vendor/vendor/qcom/proprietary/b2g_telephony/RIL.cpp

```cpp
/**
 * Send the dial request to RIL
 */
void RIL::Dial(RilResponseHandler* handler, const char* address, int clirMode,
    void* userObj) {
  RildRequest* rr = RildRequest::Obtain(RIL_REQUEST_DIAL, handler, userObj);

  WriteStringToParcel(rr->mParcel, address);
  rr->mParcel.writeInt32(clirMode);
  rr->mParcel.writeInt32(0); // UUS info is not supported

  SUB_QRLOGD("[%04d]> %s", rr->mSerial, RequestToString(rr->mRequest));
  Send(rr);
}
/**
 * Add the request to the pending request queue and send
 * the data to the rild socket using the RILSender
 * runnable on the {@code mRilSenderThread}
 */
void RIL::Send(RilRequest* rilRequest) {
  // Add the request to the pending request list
  pthread_mutex_lock(&mPendingRequestsMutex);
  mPendingRequests.insertFront(new RilRequestListItem(rilRequest));
  pthread_mutex_unlock(&mPendingRequestsMutex);

#ifdef ENABLE_TEST_FRAMEWORK
  if (mTestCallback != nullptr) {
    RefPtr<nsRunnable> runnable = new TestRequestInterceptor(this, rilRequest);
    NS_DispatchToMainThread(runnable);
  } else {
    DispatchToRil(rilRequest);
  }
#else
  DispatchToRil(rilRequest);//--> dispatch to ril
#endif
}

/**
 * Dispatch the RIL request to sender thread
 */
void RIL::DispatchToRil(RilRequest* rilRequest) {
  RefPtr<nsRunnable> event = new RilSender(this, rilRequest);
  mRilSenderThread->Dispatch(event, nsIEventTarget::DISPATCH_NORMAL);//---> Rilsender send
}

/*============= Implementation of Class RilSender =============*/

/**
 * RilSender constructor
 */
RilSender::RilSender(RIL* ril, RilRequest* rilRequest) :
      mRil(ril),
      mRilRequest(rilRequest) {
  switch (rilRequest->mSocketId)  {
    case SOCKETID_OEM:
      mSocket = ril->mOemSocket;//cs call mOemSocket = new RilSocket(..) 
      break;
    case SOCKETID_IMS:
      mSocket = ril->mImsSocket;//ps call
      break;
    default:
      mSocket = ril->mRildSocket;
  }
}

/**
 * Send data to rild socket
 */
NS_IMETHODIMP RilSender::Run() {
  uint32_t header;
  const void *data = mRilRequest->Data();
  size_t dataSize = mRilRequest->DataSize();
  int ret;

  ...
  pthread_mutex_lock(&mRil->mWriteMutex);

  // Send payload size
  header = htonl(dataSize);
  ret = BlockingWrite((void *)&header, sizeof(header));
  if (ret < 0) {
    pthread_mutex_unlock(&mRil->mWriteMutex);
    goto error;
  }

  // Send payload
  ret = BlockingWrite(data, dataSize);// 写数据到rild
  if (ret < 0) {
    pthread_mutex_unlock(&mRil->mWriteMutex);
    goto error;
  }

  pthread_mutex_unlock(&mRil->mWriteMutex);
  return NS_OK;

  ...
}


/**
 * Writes the data to the socket and blocks until all the data
 * is sent
 */
int RilSender::BlockingWrite(const void *buffer, size_t len) {
  size_t writeOffset = 0;
  const uint8_t *toWrite;

  toWrite = (const uint8_t *)buffer;

  while (writeOffset < len) {
    ssize_t written;
    do {
      written = write (mSocket->mFd, toWrite + writeOffset,
          len - writeOffset);// --->写数据
    } while (written < 0 && ((errno == EINTR) || (errno == EAGAIN)));

    if (written >= 0) {
      writeOffset += written;
    } else {   // written < 0
      QRLOGE("[SUB%d] RIL Response: unexpected error on write errno:%d",
          mRil->mClientId, errno);
      mSocket->Close();
      return -1;
    }
  }

  return 0;
}




//socket 连接的文件
/* RilSocket related defines */
NS_NAMED_LITERAL_CSTRING(RILD_SOCKET_NAME, "rild");
NS_NAMED_LITERAL_CSTRING(RILD_OEM_SOCKET_NAME, "qmux_radio/rild_oem");
NS_NAMED_LITERAL_CSTRING(IMS_SOCKET_NAME, "qmux_radio/rild_ims");
```










作者：贱贱的杨
从此你们的路上不会孤单，还有贱贱的我



---------
[1]:https://developer.mozilla.org/zh-CN/
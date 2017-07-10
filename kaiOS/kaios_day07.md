# FFOS 展讯平台 cs call 实现方案

## gaia launcher 
- Dialer.call

```javacript
call({ number = this.state.telNum } = {}) {
    ...

    dialHelper.dial(number) //--> dial number
      .then(() => {
        this.isCalling = false;
        Service.request('Dialer:hide');
      })
      .catch(() => {
        this.isCalling = false;
      });
  }
```
- DialHelper.dial

```javacript
dial(number) {
    ...

    return new Promise((resolve, reject) => {
      Service.request('chooseSim', 'call').then((cardIndex) => {
        ...

        let telephony = navigator.mozTelephony; //获取telephony
		...

        let emergencyOnly = conn.voice.emergencyCallsOnly;
        if (emergencyOnly) {
          callPromise = telephony.dialEmergency(number);// 拨打紧急号码
        } else {
          callPromise = telephony.dial(number, cardIndex);// 拨打普通号码 --> dial
        }

        ...
    });
  }

```
- gaia 调用接口 webidl 与 gecko xpcom 组件通信
## gecko 
- gecko/gecko/dom/webidl/Telephony.webidl
- 映射到 Telephony.cpp
- gecko/dom/telephony/Telephony.cpp

```cpp
// Telephony WebIDL

already_AddRefed<Promise>
Telephony::Dial(const nsAString& aNumber, const Optional<uint32_t>& aServiceId,
                ErrorResult& aRv)
{
  uint32_t serviceId = GetServiceId(aServiceId);
  RefPtr<Promise> promise = DialInternal(serviceId, aNumber, false, aRv);
  //调用 DialInterface -->dial
  return promise.forget();
}

// 调用 dial
already_AddRefed<Promise>
Telephony::DialInternal(uint32_t aServiceId, const nsAString& aNumber,
                        bool aEmergency, ErrorResult& aRv)
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(GetOwner());
  ...

  nsCOMPtr<nsITelephonyDialCallback> callback =
    new TelephonyDialCallback(GetOwner(), this, promise);

  nsresult rv = mService->Dial(aServiceId, aNumber, aEmergency, callback);
   //  mservice = ril  -->dial
  ...

  return promise.forget();
}

// static 初始化时获取mService
already_AddRefed<Telephony>
Telephony::Create(nsPIDOMWindowInner* aOwner, ErrorResult& aRv){

  nsCOMPtr<nsITelephonyService> ril = do_GetService(TELEPHONY_SERVICE_CONTRACTID);
  // ril 指向 接口 nsITelephonyService 
  ...

  RefPtr<Telephony> telephony = new Telephony(aOwner);

  telephony->mService = ril;
}
```
- gecko/dom/telephony/nsITelephonyService.idl
- 通过idl 和接口 service通信
- 由于 nsITelephonyService.idl 在末尾处进行service实例化,所以可以看出 具体的service
```javacript
%{C++
template<typename T> struct already_AddRefed;

already_AddRefed<nsITelephonyService>
NS_CreateTelephonyService();//此处实例化 使用的是Telephony.cpp 方法 
							//第一次接触这个概念，现在还在查找资料学习
%}
```
- gecko/dom/telephony/Telephony.cpp

```cpp
already_AddRefed<nsITelephonyService>
NS_CreateTelephonyService()
{
  nsCOMPtr<nsITelephonyService> service;

  if (XRE_IsContentProcess()) {
  // 如果是app进程 调用 TelephonyIPCService -->dial 使用TelephonyIPCService
    service = new mozilla::dom::telephony::TelephonyIPCService();
  } else {
#if defined(MOZ_WIDGET_GONK) && defined(MOZ_B2G_RIL)  
//预编译处理  使用TelephonyService 
    service = do_CreateInstance(GONK_TELEPHONY_SERVICE_CONTRACTID);
#endif
  }

  return service.forget();
}
```

- gecko/dom/telephony/ipc/TelephonyIPCService.cpp
- 通过IPCservice 调用dial

```cpp
NS_IMETHODIMP
TelephonyIPCService::Dial(uint32_t aClientId, const nsAString& aNumber,
                          bool aIsEmergency,
                          nsITelephonyDialCallback *aCallback)
{
  nsCOMPtr<nsITelephonyCallback> callback = do_QueryInterface(aCallback);
  return SendRequest(nullptr, callback,
                     DialRequest(aClientId, nsString(aNumber), aIsEmergency));
  //发送打电话请求-- dial
}

nsresult
TelephonyIPCService::SendRequest(nsITelephonyListener *aListener,
                                 nsITelephonyCallback *aCallback,
                                 const IPCTelephonyRequest& aRequest)
{
  ...

  // Life time of newly allocated TelephonyRequestChild instance is managed by
  // IPDL itself.
  TelephonyRequestChild* actor = new TelephonyRequestChild(aListener, aCallback);
  mPTelephonyChild->SendPTelephonyRequestConstructor(actor, aRequest);
  //TelephonyChild sendrequest
  return NS_OK;
}


```
- 具体封装在out 目录下 child 和 parent 文件 具体相关文件如下
	- out/target/product/product/../obj/objdir-gecko/ipc/ipdl/PTelephony.cpp
	- out/target/product/product/../obj/objdir-gecko/ipc/ipdl/PTelephonyChild.cpp
	- out/target/product/product/../obj/objdir-gecko/ipc/ipdl/PTelephonyparent.cpp
	- out/target/product/product/../obj/objdir-gecko/ipc/ipdl/PTelephony.h
	- out/target/product/product/../obj/objdir-gecko/ipc/ipdl/PTelephonyChild.h
	- out/target/product/product/../obj/objdir-gecko/ipc/ipdl/PTelephonyparent.h
- 通过pTelephony.ipdl 分为 TelephonyChild.cpp 和 TelephonyParent.cpp 通信 //这部分有点复杂，需要研究
- gecko/dom/telephony/ipc/PTelephony.ipdl

```cpp
//定义了一系列操作 的结构体  在out 目录生成下列方法 
...


struct DialRequest
{
  uint32_t clientId;
  nsString number;
  bool isEmergency;
};

...

union IPCTelephonyRequest
{
  EnumerateCallsRequest;
  DialRequest;
  SendUSSDRequest;
  CancelUSSDRequest;
  ConferenceCallRequest;
  SeparateCallRequest;
  HangUpConferenceRequest;
  HoldConferenceRequest;
  ResumeConferenceRequest;
  AnswerCallRequest;
  HangUpAllCallsRequest;
  HangUpCallRequest;
  RejectCallRequest;
  HoldCallRequest;
  ResumeCallRequest;
  SendTonesRequest;
};
child:
  async NotifyCallStateChanged(nsTelephonyCallInfo[] aAllInfo);

  async NotifyCdmaCallWaiting(uint32_t aClientId, IPCCdmaWaitingCallData aData);

  async NotifyConferenceError(nsString aName, nsString aMessage);

  async NotifyRingbackTone(bool aPlayRingbackTone);

  async NotifyTelephonyCoverageLosing(uint16_t aType);

  async NotifySupplementaryService(uint32_t aClientId, int32_t aCallIndex,
                                   uint16_t aNotification);

  async NotifyTtyModeReceived(uint16_t aMode);

parent:
  /**
   * Sent when the child no longer needs to use PTelephony.
   */
  async __delete__();

  /**
   * Sent when the child makes an asynchronous request to the parent.
   */
  async PTelephonyRequest(IPCTelephonyRequest request);

  async RegisterListener();

  async UnregisterListener();

  async StartTone(uint32_t aClientId, nsString aTone);

  async StopTone(uint32_t aClientId);

  sync GetHacMode()
    returns (bool aEnabled);

  async SetHacMode(bool aEnabled);

  sync GetMicrophoneMuted()
    returns (bool aMuted);

  async SetMicrophoneMuted(bool aMuted);

  sync GetSpeakerEnabled()
    returns (bool aEnabled);

  async SetSpeakerEnabled(bool aEnabled);

  sync GetTtyMode()
    returns (uint16_t aMode);

  async SetTtyMode(uint16_t aMode);

```

- gecko/dom/telephony/ipc/TelephonyParent.cpp
- 在 parent 进程收到子进程的发送dialrequest

```cpp 

bool
TelephonyParent::RecvPTelephonyRequestConstructor(PTelephonyRequestParent* aActor,
                                                  const IPCTelephonyRequest& aRequest)
{
  TelephonyRequestParent* actor = static_cast<TelephonyRequestParent*>(aActor);
  nsCOMPtr<nsITelephonyService> service = do_GetService(TELEPHONY_SERVICE_CONTRACTID);
  //通过 TELEPHONY_SERVICE_CONTRACTID 获取服务 通过idl nsITelephonyService.idl
  if (!service) {
    return NS_SUCCEEDED(actor->GetCallback()->NotifyError(NS_LITERAL_STRING("InvalidStateError")));
  }

  switch (aRequest.type()) {
  	...

    case IPCTelephonyRequest::TDialRequest: {
      const DialRequest& request = aRequest.get_DialRequest();
      service->Dial(request.clientId(), request.number(),
                    request.isEmergency(), actor->GetDialCallback());
      //通过 gecko/dom/telephony/nsITelephonyService.idl 实现 dial
      return true;
    }
    ...

  }

  return false;
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

- gecko/gecko/dom/telephony/gonk/TelephonyService.js

```javacript

  dial: function(aClientId, aNumber, aIsDialEmergency, aCallback) {
    ...

    if (this._isCdmaClient(aClientId)) {
      this._dialCall(aClientId, aNumber, undefined, aCallback);
      return;
    }
    /*MMI Code*/
  }

_dialCall: function(aClientId, aNumber, aClirMode = RIL.CLIR_DEFAULT,
                      aCallback) {

  ...
    let options = {
      isEmergency: isEmergency,
      number: aNumber,
      clirMode: aClirMode
    };

    // No active call. Dial it out directly.
    if (!this._isActive(aClientId)) {
      this._sendDialCallRequest(aClientId, options, aCallback);
      return;
    }

    ...
  }
//send to ril
  _sendDialCallRequest: function(aClientId, aOptions, aCallback) {
    this._isDialing = true;

    this._sendToRilWorker(aClientId, "dial", aOptions, response => {
      this._isDialing = false;

      if (response.errorMsg) {
        this._sendToRilWorker(aClientId, "getFailCause", null, response => {
          aCallback.notifyError(response.failCause);
        });
      } else {
        this._ongoingDial = {
          clientId: aClientId,
          isEmergency: aOptions.isEmergency,
          callback: aCallback
        };
      }
    });
  }

  _sendToRilWorker: function(aClientId, aType, aMessage, aCallback) {
     this._getClient(aClientId).sendWorkerMessage(aType, aMessage, aCallback);
  }

```
- gecko/gecko/dom/system/gonk/RadioInterfaceLayer.js

```javacript

  sendWorkerMessage: function(rilMessageType, message, callback) {
    ...

    if (callback) {
      this.workerMessenger.send(rilMessageType, message, function(response) {
        return callback.handleResponse(response);
      });
    } else {
      this.workerMessenger.send(rilMessageType, message);
    }
  }

  send: function(clientId, rilMessageType, message, callback) {
    message = message || {};

    ...

    message.rilMessageType = rilMessageType;
    this.worker.postMessage(message);//-->post to ril_worker
  }

```
- gecko/gecko/dom/system/gonk/ril_worker.js

```javacript

onmessage = function onmessage(event) {
  ContextPool.handleChromeMessage(event.data);
};


handleChromeMessage: function(aMessage) {
    let clientId = aMessage.rilMessageClientId;
    if (clientId != null) {
      let context = this._contexts[clientId];
      context.RIL.handleChromeMessage(aMessage);
      return;
    }
    ...
    method.call(this, aMessage);
  },

  handleChromeMessage: function(message) {
    ...
    let method = this[message.rilMessageType]; --> "dial"
    ...
    method.call(this, message); -->call dial
  },

  dial: function(options) {
    if (options.isEmergency) {
      options.request = RILQUIRKS_REQUEST_USE_DIAL_EMERGENCY_CALL ?
                        REQUEST_DIAL_EMERGENCY_CALL : REQUEST_DIAL;

    } else {
      options.request = REQUEST_DIAL;

      ...
    }

    this.telephonyRequestQueue.push(options.request, () => {
      let Buf = this.context.Buf;
      ...
      Buf.writeInt32(0);
      Buf.sendParcel();
    });
  },
```
- rild use socket 通信










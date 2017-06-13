# cs call flow from launch
- 由于性能的原因，现已经将dialer 调到launch 应用中

- apps/apps/launcher/src/dialer.js

```javascript
  call({ number = this.state.telNum } = {}) {
    if (this.isCalling) {
      return;
    }
    this.isCalling = true;
    this.stopRenderSteply();

    dialHelper.dial(number)
      .then(() => {
        this.isCalling = false;
        Service.request('Dialer:hide');
      })
      .catch(() => {
        this.isCalling = false;
      });
  }
```
- apps/apps/launcher/src/util/dial_helper.js

```javascript
dial(number) {
    /*
     * 字串处理
     */
    

    return new Promise((resolve, reject) => {
      Service.request('chooseSim', 'call').then((cardIndex) => {
        /**
         * 判断网络连接
         */

        let emergencyOnly = conn.voice.emergencyCallsOnly;
        if (emergencyOnly) {
          callPromise = telephony.dialEmergency(number);//-->dialEmergency
        } else {
          callPromise = telephony.dial(number, cardIndex);//-->dial 
        }

        ...	
      })
      .catch(() => { // for cancel sim card choosing
        reject();
      });
    });
  }

```
- 调用 webidl 接口 进入gecko
- gecko/gecko/dom/webidl/Telephony.webidl  后面的流程和上一篇一样


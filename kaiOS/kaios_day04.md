


# setttings

## manifest.Webapp

- 指定app 启动入口 

```json
"launch_path": "/index.html"
```

## index.html
- body 中执行 javascript 

```html
<script src="js/startup.js"></script>
```

## startup.js 包含
- global LazyLoader
- InitialPanelHandler
- RootPanelHandler
- AppStarter.starter

### 1.在自执行中 启动 AppStarter.start()

```javascript
//(function(){})(); 函数自执行格式
(function() {
  'use strict'; //严格模式

  var appStarter = AppStarter();// 引用函数

/*
 * 0-UNINITIALIZED：XML 对象被产生，但没有任何文件被加载。 
 * 1-LOADING：加载程序进行中，但文件尚未开始解析。 
 * 2-LOADED：部分的文件已经加载且进行解析，但对象模型尚未生效。 
 * 3-INTERACTIVE：仅对已加载的部分文件有效，在此情况下，对象模型是有效但只读的，可以交互。 
 * 4-COMPLETED：文件已完全加载，代表加载成功。 
 */
  if (document.readyState !== 'loading') {//没有在加载
  	/*
  	 * != 在表达式两边的数据类型不一致时,会隐式转换为相同数据类型,然后对值进行比较.
	 * !== 不会进行类型转换,在比较时除了对值进行比较以外,还比较两边的数据类型, 它是恒等运算符===的非形式.
  	 */
    appStarter.start();
  } else {
    document.addEventListener('readystatechange', function readyStateChange() {
    //监听readyStateChange（)方法
      if (document.readyState == 'interactive') {//可以交互
        document.removeEventListener('readystatechange', readyStateChange);
        appStarter.start();
      }
    });
  }
})();
```

### 2. start()

```javascript
/**
 * AppStarter determines the initial panel to be displayed for this launch. It
 * is also reponsible for attaching basic panel handlers for enabling the
 * ability of interacting with users.
 *
 * @module AppStarter
 */
(function(exports) {
  'use strict';
 //.......
   /**
   * @class AppStarter
   * @returns {AppStarter}
   */
  function AppStarter() {
    this._started = false;
    this._launchContext = null;
  }

  AppStarter.prototype = {

  	/**
     * Returns the initial panel id based on the pending system message. If
     * there is no system message available, it returns 'root'.
     *
     * @access private
     * @memberOf AppStarter.prototype
     * @returns {Promise String}
     */
    _getInitialPanelId: function as_getInitialPanelId() {
      return new Promise(function(resolve) {
        if (navigator.mozHasPendingMessage('activity')) {
          // Load activity handler only when we need to handle it.
          LazyLoader.load(['js/activity_handler.js'], function ah_loaded() {
            window.ActivityHandler.ready().then(function ah_ready() {
              resolve(window.ActivityHandler.targetPanelId);
            });
          });
        } else {
          resolve('root');
        }
      });
    },
    /**
     * The function determines the first panel to be displayed and loads the
     * minimal set of modules for basic interaction. It also exposes the launch
     * context for the delay loaded AMD modules.
     *
     * @access public
     * @memberOf AppStarter.prototype
     */
	start: function as_start() {
  	//.......

  	//包含浏览器相关信息 
  	/*
	 * mozL10n 在index.html 中引入脚本L10n.js 库 一旦载入L10n.js库，‘navigator.mozL10n‘会被客户端使用
	 * <script src="shared/js/l10n.js"></script> 
  	 */
      navigator.mozL10n.once(function l10nDone() {
      	//........
      });

	//...then()方法是异步执行 使用promise 范式
      return this._getInitialPanelId().then((initialPanelId) => {
        this._showInitialPanel(initialPanelId);

        // XXX: This is an optimization for the root panel to avoid reflow that
        //      could be observed by users.
        var customPanelHandler;
        if (initialPanelId === 'root') {
          customPanelHandler = RootPanelHandler;
        }

        var initialPanelHandler =
          InitialPanelHandler(document.getElementById(initialPanelId),
            customPanelHandler);

        // Initial panel handler registers basic events for interaction so we
        // can fire the content interactive evnet here.
        window.performance.mark('contentInteractive');
        window.dispatchEvent(new CustomEvent('moz-content-interactive'));

        this._createLaunchContext(initialPanelId, initialPanelHandler,
          window.ActivityHandler);
      }).then(() => {
        // Add timeout as loading the modules could block scrolling.
        return new Promise((resolve) => {
          setTimeout(() => {
            this._loadAlameda();//加载Alameda
            resolve();
          }, 100);
        });
      });
    }
  };

  exports.AppStarter = function ctor_appStarter() {
    return new AppStarter();//实例化
  };
})(this);

//查询 target id 来源  root

    /**
     * Returns the initial panel id based on the pending system message. If
     * there is no system message available, it returns 'root'.
     *
     * @access private
     * @memberOf AppStarter.prototype
     * @returns {Promise String}
     */
    _getInitialPanelId: function as_getInitialPanelId() {
      return new Promise(function(resolve) {
        if (navigator.mozHasPendingMessage('activity')) {
          // Load activity handler only when we need to handle it.
          LazyLoader.load(['js/activity_handler.js'], function ah_loaded() {
            window.ActivityHandler.ready().then(function ah_ready() {
              resolve(window.ActivityHandler.targetPanelId);
            });
          });
        } else {
          resolve('root');
        }
      });
    },

    /**
     * Insert the elements of the initial panel.
     *
     * @access private
     * @memberOf AppStarter.prototype
     */
    _showInitialPanel: function as_showInitialPanel(initialPanelId) {
      var initialPanel = document.getElementById(initialPanelId);
      initialPanel.classList.add('current'); //classlist 元素的类名列表
      initialPanel.innerHTML = initialPanel.firstChild.textContent;
    },
    /**
     * The function defines a launch context storing the information regarding
     * the launch to be used by the AMD modules.
     *
     * @access private
     * @memberOf AppStarter.prototype
     */
    _createLaunchContext: function as_createLaunchContext(initialPanelId,
      initialPanelHandler, activityHandler) {

      this._launchContext = {
        get initialPanelId() {
          return initialPanelId;
        },
        get initialPanelHandler() {
          return initialPanelHandler;
        },
        get activityHandler() {
          return activityHandler;
        }
      };

      var that = this;
      Object.defineProperty(exports, 'LaunchContext', {
        configurable: true,
        get: function() {
          return that._launchContext;
        }
      });
    },

    /**
     * Load alameda and the required modules defined in main.js.
     *
     * @access private
     * @memberOf AppStarter.prototype
     * demo: 　　<script src="js/require.js" data-main="js/main"></script>
     *    data-main属性的作用是，指定网页程序的主模块   加载库文件
     */
    _loadAlameda: function as_loadAlameda() {
      var scriptNode = document.createElement('script');
      scriptNode.setAttribute('data-main', 'js/main.js');//主模块
      scriptNode.src = 'js/vendor/alameda.js';//库文件
      document.head.appendChild(scriptNode);
    },

```
```javascript
//main.js

```


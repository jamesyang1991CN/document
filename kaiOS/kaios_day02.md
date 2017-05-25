# new kaios app

## preparation
- install Firefox and WEBIDE

## operation
- open WEBIDE
- connect to UE
- click new app
- input project name then click ok
- choose a folder to save
- click run 
- check UE ,hello world app open,show on screen

## introduce APP
- 由一个文件夹icons 、 app.js 、index.html、manifest.webapp 组成
- icons 存放app icon 几种尺寸的图片 有128*128 16*16 48*48 60*60
- manifest.webapp json结构的描述文件
``` 
{
	"name": "Demo", //应用的名称
	"description": "A Hello World app",
	"launch_path": "/index.html", 
	//应用的起始页位置 全都是绝对路径 例如：http://example.com／index.html launch_path 就是／index／html
	"icons": {
		"16": "/icons/icon16x16.png",
		"48": "/icons/icon48x48.png",
		"60": "/icons/icon60x60.png",
		"128": "/icons/icon128x128.png"
	},
	"developer": {
		"name": "Your name",
		"url": "http://example.com"
	}
}
```
- launch_path index.html 所以看下其实就是网页html初始化的界面
``` html
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width,user-scalable=no,initial-scale=1">

    <title>Hello World</title>

    <style>
      body {
        border: 1px solid black;
      }
    </style>

    <!-- Inline scripts are forbidden in Firefox OS apps (CSP restrictions),
         so we use a script file. -->
    <script src="app.js" defer></script>

  </head>

  <body>
    <!-- This code is in the public domain. Enjoy! -->
    <h1>Hello World</h1>
  </body>

</html>
```
- index.html加载了app.js所以下面看app.js window监听事件加载function输出到界面
``` javascript
window.addEventListener("load", function() {
  console.log("Hello World!");
});
```





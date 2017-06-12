# js 
- 常见格式：(function() { /* code */ })();
- 解释：包围函数（function(){})的第一对括号向脚本返回未命名的函数，随后一对空括号立即执行返回的未命名函数，括号内为匿名函数的参数
- 作用：可以用它创建命名空间，只要把自己所有的代码都写在这个特殊的函数包装内，那么外部就不能访问，除非你允许(变量前加上window，这样该函数或变量就成为全局)。各JavaScript库的代码也基本是这种组织形式。
- 其他格式

```javascript
（(function () { /* code */ } ()); 
!function () { /* code */ } ();
~function () { /* code */ } ();
-function () { /* code */ } ();
+function () { /* code */ } ();
```

##  jsvascrtipt 函数 和 闭包问题分析

### javascript由 ECMAscript DOM BOM 组成

#### ECMAscript
- 延迟脚本 defer属性  表明脚本执行的时候不会影响页面的构造，理论上多个脚本载入按照先后次序执行

```javascript
<script defer src="dist/vendors.js"></script>
<script defer src="dist/bundle.js"></script>
```

- 异步脚本 async属性 与defer类似，但是异步，不按照先后次序执行
- 语法
	- 区分大小写
	- 标识符 第一个字符：字母 下划线 美元符 其他：字母 数字 下划线 美元符号
	- 严格模式 "use strict" 是编译指示 告诉引擎切换到严格模式

- 函数
	- 函数格式

```javascript
function functionName(arg0,arg1,...agrN){
	statements;
}
```
	- 函数没有重载
- 函数参数 arguments对象 包含所有函数参数的数组 在函数中可以引用

- 变量 作用域 内存问题	

- 函数内部属性 arguments this
	- arguments 属性 callee 指针 指向 拥有arguments对象的函数
	- this 函数据以执行环境变量
	- caller 保存着调用当前函数的函数的引用
- 函数的属性和方法
	- length 函数接收的参数个数
	- prototype 
	- 每个函数都有两个方法 apply() call()   作用可以扩充函数的作用域
		- apply() 接收两个参数 一个在其运行的函数的作用域 另一个是参数数组
		- call() 功能类似apply()

- OOP
- 静态私有变量和方法

- BOM





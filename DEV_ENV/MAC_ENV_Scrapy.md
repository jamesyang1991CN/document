# scrapy 网络爬虫 环境搭建 (OSX)

访问 [github scrapy][1] 可以获得搭建网络爬虫的安装环境，但是需要翻墙

# 1.github 介绍了一种最快的安装命令

``` python
$ pip install scrapy
```

- 但是会遇见很多的问题，需要安装一些依赖包

# 2.[scrapy 官网][2] 提供了 guide 引导我们安装
- mac 自带 python

``` python
$ python --version
Python 2.7.10
```

- Scrapy 依赖 如下包 lxml、parsel、w3lib、twisted （我的电脑上已经安装）

如果没有安装执行命令

``` python
$ sudo pip install lxml （可替换parsel、w3lib、twisted 如果缺少）
```

# 3.安装 Homebrew，wget
- 访问 [homebrew 官网][3] 按照guide 安装 homebrew

``` python
$/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

```
- 安装wget

``` python
$ brew install wget
```

# 4.安装pip工具包

- 第一步

``` python
$ wget https://bootstrap.pypa.io/get-pip.py
```
- 第二步
``` python
$ sudo python get-pip.py
```

# 5.升级 XCODE 到最新版本

# 6. 安装Scrapy

- 安装

``` python
$ sudo pip install scrapy   
```

- 安装失败 会遇见如下错误

``` python
OSError: [Errno 1] Operation not permitted: '/var/folders/6t/h404bjcd5tb_4q86tpv_251rv_0h0j/T/pip-sYsqDS-uninstall/System/Library/Frameworks/Python.framework/Versions/2.7/Extras/lib/python/six-1.4.1-py2.7.egg-info'   
```

- 原因分析：
Scrapy依赖six库，但是系统的six库比较老，安装scrapy需要卸载之后安装一个新的。但是Mac OS本身也依赖six，导致无法删除，因此没有办法安装Scrapy。

- 解决的方式：使用virtualenv来安装

``` python
$ sudo pip install virtualenv
$ virtualenv scrapyenv
$ cd scrapyenv
$ source bin/activate
$ pip install Scrapy   
```





-------
[1]: https://github.com/scrapy/scrapy
[2]: https://doc.scrapy.org/en/latest/intro/install.html
[3]: https://brew.sh/
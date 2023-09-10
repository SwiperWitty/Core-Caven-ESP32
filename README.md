# Core-Caven-ESP32
ESP32 &amp; ESP32S3 &amp; more



### Usage

这里不会讲如何使用git！

~~~shell
git clone https://github.com/SwiperWitty/Core-Caven-ESP32.git	#克隆本工程

cd Core-Caven-ESP32							#进入这个工程

#如果网络出现问题，那么就是用这个指令（已经存在Core-Caven-ESP32文件你可以这么干）
git config --global --unset http.proxy		#取消网络代理

git branch 									#查看分支是不是在主分支
#如果不是主分支，那就切换分支
git checkout main							#切换到主分支

#如果存在子模块
git submodule update --init --recursive

cd xxxx						#进入子模块
git branch 									#查看分支是不是在主分支
#如果不是主分支，那就切换分支
git checkout main							#切换到主分支

cd ..						#回到工程，开始使用吧
~~~



### CMD

~~~shell
idf.py set-target espxx #设置目标芯片

idf.py clean
idf.py fullclean 		#清除之前的编译

idf.py build 			#编译

idf.py -p COMx monitor 	#打开串口监视器
idf.py -p COMx flash 	#向串口烧录固件
idf.py -p COMx flash monitor
idf.py menuconfig 		#配置菜单

~~~



### Menu

~~~txt
txt

~~~



### Question

~~~txt
1. 接上串口助手就不运行了
	答：垃圾串口助手，他连接时会控制RTS(EN),DTR(IO0),导致esp32进入下载模式或者不运行，建议使用IDF的monitor指令。
2. 为啥编译不通过？或者不能清除?
	答：看一眼sdkconfig文件的目标芯片对不对，然后[idf.py fullclean]->[idf.py build]->[idf.py -p COMx flash monitor]试一下。

......
~~~

如果遇到其他问题：
[点击这里](https://www.baidu.com/) 



### Author

[卡文迪许怪](https://github.com/SwiperWitty) 

[Github](https://github.com/SwiperWitty) 

[bilibili](https://space.bilibili.com/102898291?spm_id_from=333.1007.0.0) （这里能找到up） 




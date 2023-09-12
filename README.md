# Core-Caven-ESP32
ESP32 &amp; ESP32S3 &amp; more



____



### Usage

这里不会讲如何使用git

~~~shell
git clone https://github.com/SwiperWitty/Core-Caven-ESP32.git 	#克隆本工程

cd Core-Caven-ESP32 	#进入这个工程

#如果网络出现问题，那么就是用这个指令（已经存在Core-Caven-ESP32文件你可以这么干）
git config --global --unset http.proxy 	#取消网络代理

git branch 			#查看分支是不是在主分支
#如果不是主分支，那就切换分支
git checkout main 	#切换到主分支

#如果存在子模块
git submodule update --init --recursive

cd xxxx 			#进入子模块
git branch 			#查看分支是不是在主分支
#如果不是主分支，那就切换分支
git checkout main 	#切换到主分支

cd .. 				#回到工程，开始使用吧
~~~



____



### CMD

~~~shell
idf.py set-target espxx #设置目标芯片

idf.py clean
idf.py fullclean 		#清除之前的编译

idf.py build 			#编译

idf.py -p comx monitor 	#打开串口监视器
idf.py -p comx flash 	#向串口烧录固件
idf.py -p comx flash monitor
idf.py menuconfig 		#配置菜单

~~~



____



### Menu

~~~txt
txt

~~~



____



### Question

~~~txt
1. 接上串口助手就不运行了?
	答：垃圾串口助手，他连接时会控制RTS(EN)和DTR(IO0),导致esp32进入下载模式或者不运行，祥见end说明附页[1]。
2. 为啥编译不通过？或者不能清除?
	答：看一眼sdkconfig文件的目标芯片对不对，然后在idf CMD打开你的工程路径使用：[idf.py fullclean]->[idf.py build]->[idf.py -p COMx flash monitor]试一下。

......
~~~

如果遇到其他问题：
[点击这里](https://www.baidu.com/) 



____



### Author

[卡文迪许怪](https://github.com/SwiperWitty) 

[Github](https://github.com/SwiperWitty) 

[bilibili](https://space.bilibili.com/102898291?spm_id_from=333.1007.0.0) （这里能找到up） 



### End

#### 说明附页

##### [1]CH343掉大坑（2023.9.10）

问题说明：ch343在连接**电脑串口软件**时，RTS（芯片复位脚RST）会给一个复位脉冲1->0->1，DTR（启动位置脚BOOT0/IO0）会变成0，如果是MCU那就没错（MCU的BOOT0 = 0是flash启动），但是ESP32就歇逼了（ESP32的IO = 0是烧入模式），那就出大问题。

![image-20230911142822254](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309111428325.png)

接下来就是如何让**电脑串口软件**连接ch343不去控制DTR&RTS呢？

首先建议1：换 [沁恒官方串口软件](https://www.wch.cn/downloads/COMTransmit_ZIP.html) ，你可以直接解决问题。

but，他这个串口软件不支持数据颜色啊，esp32消息那么多，没有颜色不痛苦了吗?那只能继续看了。

其次建议2：改驱动(每接一个ch343设备都要改)。

![image-20230911141649967](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309111416058.png)

如果你没有这个选项，那么说明你的驱动不对/太老了。

解决方案：下载驱动并重装。[驱动地址](https://www.wch.cn/downloads/CH343SER_EXE.html) 

![image-20230911141948058](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309111419095.png)



这样，这个坑就填上了，串口软件无法流控了。依旧可以自动下载。

![image-20230911142027388](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309111420471.png)











#### 菜鸡看这里

##### [1]改工程名称

![image-20230912212449769](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309122124809.png)



如果你想给这个工程添加其他 `*.c`和`*.h`文件我建议你放在工程路径`main\driver\src`(放.c)`main\driver\inc`(放.h) 下。放了之后，请把这些文件添加到`main\CMakeLists.txt`里，参加编译，不然就是白写。

![image-20230912213304558](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309122133600.png)





##### [2]使用下载工具

如图配置

![toolset](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309122136233.png)





_____

它是有底线的


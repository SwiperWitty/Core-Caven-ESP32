# Hello World Example

首先，这个文件源于乐鑫IDF 4.4.5 `Espressif\frameworks\esp-idf-v4.4.5\examples\get-started\hello_world` 



其次，我简单汪两句（给小白）：

拿到这个文件用vscode打开你会发现此文件头文件一片红，那是因为你没让vscode知道你参考库的位置。

![image-20230913112604984](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131128696.png)

how to do it ？



vscode打开工程文件夹，使用按住`ctrl` `shift` 再按 `p` ，你就会触发如图所示，接下来在这个出现的框里输入`ESP-IDF:Add vscod`，选择如图所示即可。

![image-20230913112025350](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131120428.png)



此工程文件夹下的`.vscode`就会跟新，如图。

![image-20230913112349704](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131128627.png)



这样你的头文件就不会报红了。

![image-20230913112752519](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131127561.png)



如果还有问题，[请点击我](https://www.baidu.com/) 谢谢！

_____



Starts a FreeRTOS task to print "Hello World".

(See the README.md file in the upper level 'examples' directory for more information about examples.)

## How to use example

Follow detailed instructions provided specifically for this example. 

Select the instructions depending on Espressif chip installed on your development board:

- [ESP32 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/stable/get-started/index.html)
- [ESP32-S2 Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s2/get-started/index.html)


## Example folder contents

The project **hello_world** contains one source file in C language [hello_world_main.c](main/hello_world_main.c). The file is located in folder [main](main).

ESP-IDF projects are built using CMake. The project build configuration is contained in `CMakeLists.txt` files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both). 

Below is short explanation of remaining files in the project folder.

```
├── CMakeLists.txt
├── example_test.py            Python script used for automated example testing
├── main
│   ├── CMakeLists.txt
│   ├── component.mk           Component make file
│   └── hello_world_main.c
├── Makefile                   Makefile used by legacy GNU Make
└── README.md                  This is the file you are currently reading
```

For more information on structure and contents of ESP-IDF projects, please refer to Section [Build System](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/build-system.html) of the ESP-IDF Programming Guide.

## Troubleshooting

* Program upload failure

    * Hardware connection is not correct: run `idf.py -p PORT monitor`, and reboot your board to see if there are any output logs.
    * The baud rate for downloading is too high: lower your baud rate in the `menuconfig` menu, and try again.

## Technical support and feedback

Please use the following feedback channels:

* For technical queries, go to the [esp32.com](https://esp32.com/) forum
* For a feature request or bug report, create a [GitHub issue](https://github.com/espressif/esp-idf/issues)

We will get back to you as soon as possible.

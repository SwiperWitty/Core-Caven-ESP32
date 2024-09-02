# FreeRTOS Real Time Stats Example

首先，这个文件源于乐鑫IDF 4.4.5 `Espressif\frameworks\esp-idf-v4.4.5\examples\system\freertos\real_time_stats` 



其次，我简单汪两句（给小白）：

拿到这个文件用vscode打开你会发现此文件头文件一片红，那是因为你没让vscode知道你参考库的位置。

![image-20230913112604984](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131126036.png)

how to do it ？



vscode打开工程文件夹，使用按住`ctrl` `shift` 再按 `p` ，你就会触发如图所示，接下来在这个出现的框里输入`ESP-IDF:Add vscod`，选择如图所示即可。

![image-20230913112025350](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131120428.png)



此工程文件夹下的`.vscode`就会跟新，如图。

![image-20230913112349704](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131123748.png)



这样你的头文件就不会报红了。

![image-20230913112752519](https://gitee.com/Swiper_witty/caven_img/raw/master/img/202309131127561.png)



如果还有问题，[请点击我](https://www.baidu.com/) 谢谢！

_____



(See the README.md file in the upper level 'examples' directory for more information about examples.)

FreeRTOS provides the function `vTaskGetRunTimeStats()` to obtain CPU usage statistics of tasks. However, these statistics are with respect to the entire runtime of FreeRTOS (i.e. **run time stats**). Furthermore, statistics of `vTaskGetRunTimeStats()` are only valid whilst the timer for run time statistics has not overflowed.

This example demonstrates how to get CPU usage statistics of tasks with respect to a specified duration (i.e. **real time stats**) rather than over the entire runtime of FreeRTOS. The `print_real_time_stats()` function of this example demonstrates how this can be achieved.

## How to use example

### Hardware Required

This example should be able to run on any commonly available ESP32 development board.

### Configure the project

```
idf.py menuconfig
```

* Select `Enable FreeRTOS to collect run time stats` under `Component Config > FreeRTOS` (this should be enabled in the example by default)

* `Choose the clock source for run time stats` configured under `Component Config > FreeRTOS`. The `esp_timer` should be selected be default. This option will affect the time unit resolution in which the statistics are measured with respect to.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

The example should have the following log output:

```
...
Getting real time stats over 100 ticks
| Task | Run Time | Percentage
| stats | 1304 | 0%
| IDLE0 | 206251 | 10%
| IDLE1 | 464785 | 23%
| spin2 | 225389 | 11%
| spin0 | 227174 | 11%
| spin4 | 225303 | 11%
| spin1 | 207264 | 10%
| spin3 | 225331 | 11%
| spin5 | 225369 | 11%
| Tmr Svc | 0 | 0%
| esp_timer | 0 | 0%
| ipc1 | 0 | 0%
| ipc0 | 0 | 0%
Real time stats obtained
...
```

## Example Breakdown

### Spin tasks

During the examples initialization process, multiple `spin` tasks are created. These tasks will simply spin a certain number of CPU cycles to consume CPU time, then block for a predetermined period.

### Understanding the stats

From the log output, it can be seen that the spin tasks consume nearly an equal amount of time over the specified stats collection period of `print_real_time_stats()`. The real time stats also display the CPU time consumption of other tasks created by default in ESP-IDF (e.g. `IDLE` and `ipc` tasks).
#project(ESP-Cavend)    #上一级已包含

# 【#】开头是屏蔽的一行，空行无效
#add_subdirectory(driver)
#include_directories(driver/inc)
#link_directories(driver/src)

# 所有参与编译的 [*.c]文件
set(srcs    
    "User_app/main.c"           # [app_main] 程序入口所在

    # "driver/src/default.c"
    "driver/src/information.c"
    "driver/src/info_frame_handle.c"
    "driver/src/rtc_time.c"
    "driver/src/Custom_gpio.c"
    "driver/src/Custom_uart.c"
    "driver/src/lcd_st7789.c"
    "driver/src/draw.c"
)

# [*.c]文件合集 + [*.h]头文件
idf_component_register(SRCS  
    "${srcs}"
    INCLUDE_DIRS "User_app"		#头文件
    INCLUDE_DIRS "driver/inc"	#头文件
 
)

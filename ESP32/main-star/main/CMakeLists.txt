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
    "driver/src/Custom_uart.c"
    # "driver/src/Custom_bluetooth.c"
    "driver/src/Network_manage.c"
    "driver/src/tcp_server_link.c"
    "driver/src/tcp_client_link.c"
    "driver/src/https.c"
    "driver/src/Custom_gpio.c"

    "driver/src/Mode_RTC8564.c"
    "driver/src/MODE_st7789_dever.c"
    "driver/src/MODE_LCD.c"
    "driver/src/draw.c"
    
)

file(GLOB_RECURSE SOURCE_FILE2 "GUI/*.c")
list(APPEND srcs ${SOURCE_FILE2})

# [*.c]文件合集 + [*.h]头文件
idf_component_register(SRCS  
    "${srcs}"
    INCLUDE_DIRS "User_app"		#头文件
    INCLUDE_DIRS "driver/inc"	#头文件
    
    INCLUDE_DIRS "GUI"		    #头文件
    INCLUDE_DIRS "GUI/ui"		#头文件
    INCLUDE_DIRS "GUI/ui/components"
    INCLUDE_DIRS "GUI/lvgl"		#头文件
    INCLUDE_DIRS "GUI/lvgl/src/core"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "server_certs/server_root_cert.pem" "server_certs/local_server_cert.pem"
)

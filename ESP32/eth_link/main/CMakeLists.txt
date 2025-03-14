#project(ESP-Cavend)    #上一级已包含

# 【#】开头是屏蔽的一行，空行无效
#add_subdirectory(driver)
#include_directories(driver/inc)
#link_directories(driver/src)

# 所有参与编译的 [*.c]文件
set(srcs
    # "driver/src/default.c"
    "driver/src/information.c"
    "driver/src/info_frame_handle.c"
    "driver/src/MODE_uart.c"
    "driver/src/MODE_gpio.c"
    # "driver/src/Custom_bluetooth.c"
    "driver/src/Network_manage.c"
    "driver/src/tcp_server_link.c"
    "driver/src/tcp_client_link.c"
    "driver/src/http.c"
    "driver/src/https.c"

    "driver/src/MODE_RTC8564.c"
    "driver/src/MODE_st7789_dever.c"
    "driver/src/MODE_LCD.c"
    "driver/src/MODE_BMI088.c"
    "driver/src/draw.c"

    "API/Encrypt_crc.c"
    "API/Caven_info_frame.c"
    "API/caven_at_info_frame.c"

    "User_app/main.c"           # [app_main] 程序入口所在
    "User_app/show_app.c"
    "User_app/system_app.c"
    "User_app/Message_info_app.c"
    
)

file(GLOB_RECURSE SOURCE_FILE2 "GUI/*.c")
list(APPEND srcs ${SOURCE_FILE2})

# [*.c]文件合集 + [*.h]头文件
idf_component_register(SRCS  
    "${srcs}"
    INCLUDE_DIRS "User_app"		#头文件
    INCLUDE_DIRS "API"	        #头文件
    INCLUDE_DIRS "driver/inc"	#头文件
    
    INCLUDE_DIRS "GUI"		    #头文件
    INCLUDE_DIRS "GUI/ui"		#头文件
    INCLUDE_DIRS "GUI/ui/components"
    INCLUDE_DIRS "GUI/lvgl"		#头文件
    INCLUDE_DIRS "GUI/lvgl/src/core"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "server_certs/server_root_cert.pem" "server_certs/local_server_cert.pem"
)

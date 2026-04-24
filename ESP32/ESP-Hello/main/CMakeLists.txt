#project(ESP-Cavend)    #上一级已包含

# 【#】开头是屏蔽的一行，空行无效
#add_subdirectory(driver)
#include_directories(driver/inc)
#link_directories(driver/src)


# 所有参与编译的 [*.c]文件
set(srcs
    "API/API.c"
    "API/Time_Handle.c"
    "API/Data_Handle.c"
    "API/Caven_info_frame.c"
    "API/Caven_at_info_frame.c"
    "API/Encrypt_crc.c"

    "esp_driver/src/Base_Sys_Time.c"
    "esp_driver/src/Base_UART.c"
    "esp_driver/src/Base_GPIO.c"
    "esp_driver/src/Base_SPI.c"
    "esp_driver/src/Base_IIC.c"

    "Mode/Mode.c"
    "Mode/MODE_Time.c"
    "Mode/MODE_UART.c"

    "User_app/main.c"           # [main] 程序入口所在
    "User_app/show_app.c"
)

# file(GLOB_RECURSE SOURCE_FILE2 "GUI/*.c")
list(APPEND srcsa ${SOURCE_FILE2})

# [*.c]文件合集 + [*.h]头文件
idf_component_register(SRCS  
    "${srcs}"
    INCLUDE_DIRS "User_app"		#头文件
    INCLUDE_DIRS "API"	        #头文件
    INCLUDE_DIRS "Mode"
    INCLUDE_DIRS "esp_driver/inc"	#头文件

    INCLUDE_DIRS "."
    EMBED_TXTFILES "server_certs/server_root_cert.pem" "server_certs/local_server_cert.pem"
)

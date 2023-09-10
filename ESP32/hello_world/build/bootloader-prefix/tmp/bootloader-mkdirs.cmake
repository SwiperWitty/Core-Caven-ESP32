# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Espressif/frameworks/esp-idf-v4.4.5/components/bootloader/subproject"
  "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader"
  "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader-prefix"
  "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader-prefix/tmp"
  "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader-prefix/src/bootloader-stamp"
  "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader-prefix/src"
  "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "E:/Sea_of_Stars/Software/Core-Caven-ESP32/ESP32/hello_world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()

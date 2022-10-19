# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/monto/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader"
  "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader-prefix"
  "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader-prefix/tmp"
  "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader-prefix/src"
  "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/monto/OneDrive/Documents/esp32/esp-idf-v4.4/examples/get-started/blink/main/blink/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()

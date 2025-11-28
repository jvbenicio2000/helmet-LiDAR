# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# Se você estiver usando Pico W:
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(helmet_lidar C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# ------------ Executável principal (sistema de detecção de obstáculos) -------------
add_executable(main
    main.c
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

target_link_libraries(main
    pico_stdlib
    hardware_pwm
    hardware_gpio
)

pico_add_extra_outputs(main)

# ------------ Executável aprimorado (sistema de detecção com filtro) -------------
add_executable(main_enhanced
    main_enhanced.c
)

pico_enable_stdio_usb(main_enhanced 1)
pico_enable_stdio_uart(main_enhanced 0)

target_link_libraries(main_enhanced
    pico_stdlib
    hardware_pwm
    hardware_gpio
)

pico_add_extra_outputs(main_enhanced)

# ------------ Executável GPS/Bluetooth -------------
add_executable(gps_bluetooth
    gps_bluetooth.c
)

pico_enable_stdio_usb(gps_bluetooth 1)
pico_enable_stdio_uart(gps_bluetooth 0)

target_link_libraries(gps_bluetooth
    pico_stdlib
    hardware_uart
    hardware_gpio
)

# Se for Pico W, manter isso (mesmo sem usar Wi-Fi)
if (PICO_CYW43_SUPPORTED)
    target_link_libraries(gps_bluetooth pico_cyw43_arch_none)
endif()

pico_add_extra_outputs(gps_bluetooth)

# (Opcional) URL do projeto
pico_set_program_url(main "https://github.com/jvbenicio2000/helmet-LiDAR")
pico_set_program_url(main_enhanced "https://github.com/jvbenicio2000/helmet-LiDAR")
pico_set_program_url(gps_bluetooth "https://github.com/jvbenicio2000/helmet-LiDAR")

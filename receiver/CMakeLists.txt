cmake_minimum_required(VERSION 3.16)

# Use shared config + component-specific config
set(SDKCONFIG_DEFAULTS "../sdkconfig.defaults.common" "sdkconfig.defaults")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lap_timer_receiver)

cmake_minimum_required(VERSION 3.22.1)
project(vkandroidtest)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ANDROID_SOURCES src/main.cpp src/vulkan_renderer.cpp)
#set(IMGUI_SOURCES imgui/imgui.cpp imgui/imgui_demo.cpp imgui/imgui_draw.cpp imgui/imgui_impl_android.cpp imgui/imgui_impl_opengl3.cpp imgui/imgui_tables.cpp imgui/imgui_widgets.cpp)

set(${CMAKE_C_FLAGS} "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
set(CMAKE_CXX_STANDARD 23)

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

add_library(${CMAKE_PROJECT_NAME} SHARED ${ANDROID_SOURCES} ${IMGUI_SOURCES})


target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue include ${ANDROID_LIB_PATH}/include . )

target_link_libraries(${PROJECT_NAME} PUBLIC
        vulkan
        native_app_glue
        android
        log)
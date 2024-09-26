cmake_minimum_required (VERSION 3.8)
project ("sylar1")

# 寻找 OpenSSL 库
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# 将 src 目录下的所有源文件存放到变量 SRC_DIR 中
aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_DIR)
add_executable (${PROJECT_NAME} "main.cpp" ${SRC_DIR})

# 链接 yaml-cpp 库
target_link_libraries(${PROJECT_NAME} yaml-cpp)
# 链接 openssl 库
target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
# 链接 jsoncpp 库
target_link_libraries(${PROJECT_NAME} jsoncpp)
# 链接 ZLIB 库
target_link_libraries(${PROJECT_NAME} ZLIB::ZLIB)
# 链接 include 中的头文件
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include)
# 链接 test 中的测试文件
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/test) 
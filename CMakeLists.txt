cmake_minimum_required(VERSION 3.24)

project(Game2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/FS)
endif()

# 1. BẬT CHẾ ĐỘ LIÊN KẾT TĨNH (STATIC) CHO SFML
set(SFML_STATIC_LIBRARIES TRUE)

# 2. Trỏ trực tiếp đến thư mục SFML
set(SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/SFML-3.1.0/lib/cmake/SFML")

find_package(SFML 3.1 CONFIG REQUIRED
    COMPONENTS
        Graphics
        Audio
        Window
        System
)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(Game2D ${SOURCES})

# 3. Ép file Game2D.exe xuất ra NGAY THƯ MỤC GỐC
set_target_properties(Game2D PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}"
)

# 4. Định nghĩa macro STATIC để SFML hiểu là không dùng DLL
target_compile_definitions(Game2D PRIVATE SFML_STATIC)

target_include_directories(Game2D PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(Game2D PRIVATE
    SFML::Graphics
    SFML::Audio
    SFML::Window
    SFML::System
)

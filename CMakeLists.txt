cmake_minimum_required(VERSION 3.16)
project(BalloonTyper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Поиск SFML 3
# В версии 3.0 рекомендуется использовать конфигурацию SFMLConfig.cmake, 
# которую предоставляет сам Homebrew.
find_package(SFML 3.0 COMPONENTS Graphics Window System Audio REQUIRED)

# 2. Собираем файлы
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(BalloonTyper ${SOURCES} ${HEADERS})

# 3. Линковка (Важно: в SFML 3 компоненты пишутся с большой буквы)
target_link_libraries(BalloonTyper
    SFML::Graphics
    SFML::Window
    SFML::System
    SFML::Audio
)

# 4. Копирование ресурсов в папку с программой
add_custom_command(TARGET BalloonTyper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
)
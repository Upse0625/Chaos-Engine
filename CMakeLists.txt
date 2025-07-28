# Versión mínima de CMake requerida para este proyecto.
cmake_minimum_required(VERSION 3.10)

# Define el nombre del proyecto.
project(Chaos)

# Establece el estándar C++ a C++17 y lo hace obligatorio.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Define los directorios donde el compilador buscará los archivos de cabecera (.h).
include_directories(
    ${CMAKE_SOURCE_DIR}/lib/glad/include
    ${CMAKE_SOURCE_DIR}/lib/glfw/include
    ${CMAKE_SOURCE_DIR}/lib/glm
    ${CMAKE_SOURCE_DIR}/lib/stb
)

# Define la lista de archivos fuente (.cpp, .c) que componen tu ejecutable.
set(SOURCE_FILES
    src/main.cpp
    src/Shader.cpp
    lib/glad/src/glad.c
)

# Crea el ejecutable final a partir de los archivos fuente.
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# --- ENLAZADO DE LIBRERÍAS (LINKING) ---
if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/lib/glfw/lib-vc2022/glfw3.lib
        OpenGL32.lib
    )
else()
    message(FATAL_ERROR "Este CMakeLists.txt está configurado solo para Windows.")
endif()

# --- COPIAR ASSETS Y CONFIGURAR DIRECTORIO DE TRABAJO ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copiando carpeta de assets al directorio de salida"
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

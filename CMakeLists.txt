cmake_minimum_required(VERSION 3.15)
project(DinoFinder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detectar sistema operacional
if(WIN32)
    set(PLATFORM "Windows")
    set(OUTPUT_NAME "DinoFinder")
else()
    set(PLATFORM "Linux")
    set(OUTPUT_NAME "DinoFinder")
endif()

# ArkAPI path (deve estar no mesmo nível ou ajustar conforme necessário)
if(NOT DEFINED ARKAPI_PATH)
    set(ARKAPI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../ArkApi")
endif()

# Verificar se ArkApi existe
if(NOT EXISTS "${ARKAPI_PATH}")
    message(FATAL_ERROR "ArkApi não encontrado em ${ARKAPI_PATH}. Clone o repositório ArkApi primeiro.")
endif()

# Incluir ArkApi
include_directories("${ARKAPI_PATH}/include")
include_directories("${ARKAPI_PATH}/src")

# Arquivos do plugin
set(SOURCES
    Source/DinoFinder.cpp
)

set(HEADERS
    Source/DinoFinder.h
)

# Criar biblioteca compartilhada
if(WIN32)
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")
else()
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
    set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".so")
endif()

# Links necessários
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        "${ARKAPI_PATH}/lib/ArkApi.lib"
    )
else()
    target_link_libraries(${PROJECT_NAME} 
        "${ARKAPI_PATH}/lib/libArkApi.so"
    )
endif()

# Definir diretório de output
set_target_properties(${PROJECT_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

message(STATUS "Building DinoFinder for ${PLATFORM}")
message(STATUS "Output: ${OUTPUT_NAME}")

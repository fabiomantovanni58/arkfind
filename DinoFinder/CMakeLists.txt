cmake_minimum_required(VERSION 3.15)
project(DinoFinder)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Definir o caminho para o ArkAPI e headers do Unreal Engine
# Você precisa ajustar estes caminhos conforme sua instalação local
set(ARK_API_PATH "${CMAKE_SOURCE_DIR}/../ArkApi" CACHE PATH "Caminho para ArkAPI")
set(UE4_HEADERS_PATH "${CMAKE_SOURCE_DIR}/../UE4Headers" CACHE PATH "Caminho para headers do Unreal Engine")

# Opções de compilação
option(BUILD_64BIT "Build 64-bit" ON)

if(BUILD_64BIT)
    set(ARCH_FLAGS "-m64")
else()
    set(ARCH_FLAGS "-m32")
endif()

# Flags de compilação específicos para ARK plugins
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH_FLAGS} -fPIC -shared")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ARCH_FLAGS}")

# Incluir diretórios
include_directories(
    "${ARK_API_PATH}/Include"
    "${UE4_HEADERS_PATH}"
    "${CMAKE_SOURCE_DIR}/Source"
)

# Arquivos fonte
set(SOURCES
    Source/DinoFinder.cpp
)

# Criar biblioteca compartilhada (DLL/.so)
add_library(DinoFinder SHARED ${SOURCES})

# Configurar nome de saída
set_target_properties(DinoFinder PROPERTIES
    PREFIX ""
    OUTPUT_NAME "DinoFinder"
)

# Platform-specific output name
if(WIN32)
    set_target_properties(DinoFinder PROPERTIES SUFFIX ".dll")
elseif(APPLE)
    set_target_properties(DinoFinder PROPERTIES SUFFIX ".dylib")
else()
    set_target_properties(DinoFinder PROPERTIES SUFFIX ".so")
endif()

# Mensagem de instrução
message(STATUS "===========================================")
message(STATUS "Plugin DinoFinder configurado!")
message(STATUS "Para compilar:")
message(STATUS "  1. Clone o repositório ArkAPI")
message(STATUS "  2. Ajuste as variáveis ARK_API_PATH e UE4_HEADERS_PATH")
message(STATUS "  3. Execute: cmake -B build -S .")
message(STATUS "  4. Execute: cmake --build build")
message(STATUS "===========================================")

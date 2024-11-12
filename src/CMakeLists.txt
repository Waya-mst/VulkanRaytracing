# FindPackage
find_package(Vulkan     REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3       REQUIRED)

message(STATUS "env")

add_executable( ${PROJECT_NAME}-src)

target_compile_features(${PROJECT_NAME}-src PRIVATE cxx_std_20)
target_compile_options (${PROJECT_NAME}-src PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus /utf-8>)
target_compile_definitions( ${PROJECT_NAME}-src PRIVATE SHADER_DIR="${CMAKE_SOUCE_DIR}/src/shaders")

target_sources ( ${PROJECT_NAME}-src PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp )

target_link_libraries( ${PROJECT_NAME}-src PRIVATE Vulkan::Vulkan glm::glm glfw  )

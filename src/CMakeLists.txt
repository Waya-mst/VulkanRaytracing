set(SHADER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders/")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
# FindPackage
find_package(Vulkan     REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3       REQUIRED)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/raygen.rgen.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} -c ${SHADER_ROOT_DIR}/raygen.rgen -o ${CMAKE_CURRENT_BINARY_DIR}/raygen.rgen.spv --target-env=vulkan1.2
	DEPENDS ${SHADER_ROOT_DIR}/raygen.rgen
	COMMENT "Compiling raygen.rgen"
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/closesthit.rchit.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} -c ${SHADER_ROOT_DIR}/closesthit.rchit -o ${CMAKE_CURRENT_BINARY_DIR}/closesthit.rchit.spv --target-env=vulkan1.2
	DEPENDS ${SHADER_ROOT_DIR}/closesthit.rchit
	COMMENT "Compiling closesthit.rchit"
)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/miss.rmiss.spv
	COMMAND ${Vulkan_GLSLC_EXECUTABLE} -c ${SHADER_ROOT_DIR}/miss.rmiss -o ${CMAKE_CURRENT_BINARY_DIR}/miss.rmiss.spv --target-env=vulkan1.2
	DEPENDS ${SHADER_ROOT_DIR}/miss.rmiss
	COMMENT "Compiling miss.rmiss"
)

add_custom_target(
    compile_shaders ALL
    DEPENDS 
        ${CMAKE_CURRENT_BINARY_DIR}/raygen.rgen.spv
        ${CMAKE_CURRENT_BINARY_DIR}/closesthit.rchit.spv
        ${CMAKE_CURRENT_BINARY_DIR}/miss.rmiss.spv
)

add_executable( ${PROJECT_NAME}-src main.cpp)

target_include_directories(${PROJECT_NAME}-src PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_compile_features(${PROJECT_NAME}-src PRIVATE cxx_std_20)
target_compile_options (${PROJECT_NAME}-src PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus /utf-8>)


add_dependencies(${PROJECT_NAME}-src compile_shaders)

target_link_libraries( ${PROJECT_NAME}-src PRIVATE Vulkan::Vulkan glm::glm glfw  )

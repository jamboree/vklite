cmake_minimum_required(VERSION 3.15)
project(Vklite LANGUAGES CXX)

# all the options for this project
option(VKLITE_RUN_GENERATOR "Run the generator" OFF)
option(VKLITE_GENERATOR_BUILD "Build the generator" ON)

# Build XmlBin and Vulkan generators
if(VKLITE_GENERATOR_BUILD)
	find_package(tinyxml2 CONFIG REQUIRED)
	find_package(Boost 1.82 REQUIRED)

	# look for the file vk.xml, the ultimate source of truth for vulkan, to generate the headers from
	if(NOT DEFINED VulkanRegistry_DIR)
		if(DEFINED VKLITE_VULKAN_HEADERS_SRC_DIR)
			set(VulkanRegistry_DIR "${VKLITE_VULKAN_HEADERS_SRC_DIR}/registry")
		else()
			set(VulkanRegistry_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vulkan-Headers/registry")
		endif()
	endif()

	# The XmlBinGenerator executable
	add_executable(XmlBinGenerator XmlBinGenerator.cpp)
	target_compile_features(XmlBinGenerator PRIVATE cxx_std_20)
	target_link_libraries(XmlBinGenerator PRIVATE tinyxml2::tinyxml2)

	# The VulkanGenerator executable
	add_executable(VulkanGenerator VulkanGenerator.cpp)
	target_compile_features(VulkanGenerator PRIVATE cxx_std_20)
	target_link_libraries(VulkanGenerator PRIVATE Boost::headers)
endif()

# if the generators are to be run, add a custom commands and targets
if(VKLITE_RUN_GENERATOR)
	file(TO_NATIVE_PATH ${VulkanRegistry_DIR}/vk.xml vk_xml)
	file(TO_NATIVE_PATH ${CMAKE_CURRENT_BINARY_DIR}/vk.bin vk_bin)
	file(TO_NATIVE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/vklite/vulkan.hpp vulkan_hpp)

	add_custom_command(
		COMMAND XmlBinGenerator "${vk_xml}" "${vk_bin}"
		OUTPUT "${vk_bin}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "run XmlBinGenerator"
		DEPENDS XmlBinGenerator "${vk_xml}")
	add_custom_target(build_vk_bin ALL DEPENDS "${vk_bin}" "${vk_xml}")

	add_custom_command(
		COMMAND VulkanGenerator "${vk_bin}" "${vulkan_hpp}"
		OUTPUT "${vulkan_hpp}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "run VulkanGenerator"
		DEPENDS VulkanGenerator "${vk_bin}")
	add_custom_target(build_vulkan_hpp ALL DEPENDS "${vulkan_hpp}" "${vk_bin}")
endif()

# Create Vulkan-Hpp interface target
add_library(VkliteHeaders INTERFACE)
add_library(Vklite::Headers ALIAS VkliteHeaders)
target_compile_features(VkliteHeaders INTERFACE cxx_std_20)
target_include_directories(VkliteHeaders INTERFACE include)

if(VKLITE_RUN_GENERATOR)
	add_dependencies(VkliteHeaders build_vk_bin build_vulkan_hpp)
endif()
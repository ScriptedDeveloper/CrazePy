cmake_minimum_required(VERSION 3.26)
project(CrazePy)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCES "lexer/*.cpp" "parser/*.cpp" "*.cpp" "error_handeling/*.cpp")

add_executable(CrazePy ${SOURCES})

if(CMAKE_HOST_WIN32)
	set(CMAKE_CXX_FLAGS "/W4 /Wall /EHsc /MT /Zi /DDEBUG")
elseif(CMAKE_HOST_LINUX) # not sure if g++ is on other unix flavours
	set(CMAKE_CXX_FLAGS "-Wall -O3 -Wdeprecated -Wshadow -Wextra -Wall -Werror -Wunused -Wuninitialized")
	add_custom_target(clang-format
		COMMAND clang-format -i ${SOURCES}
		COMMENT "Running clang-format"
	)
	add_dependencies(CrazePy clang-format)
endif()






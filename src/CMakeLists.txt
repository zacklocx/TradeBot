project(TradeBot)

set(TB_VERSION_MAJOR 1)
set(TB_VERSION_MINOR 0)

set(TB_DEBUG 0)

configure_file(
	"${PROJECT_BINARY_DIR}/TradeBotConfig.h.in"
	"${PROJECT_SOURCE_DIR}/TradeBotConfig.h"
)

add_definitions(-DAVHTTP_ENABLE_ZLIB)
add_definitions(-DAVHTTP_ENABLE_OPENSSL)

find_package(zlib REQUIRED)
find_package(Boost COMPONENTS locale date_time thread filesystem system program_options regex REQUIRED)
find_package(openssl REQUIRED)
find_package(jsoncpp REQUIRED)

set(EXTRA_LIBS ${EXTRA_LIBS} ${ZLIB_LIBRARIES})
set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_LIBRARIES})
set(EXTRA_LIBS ${EXTRA_LIBS} ${OPENSSL_LIBRARIES})
set(EXTRA_LIBS ${EXTRA_LIBS} ${JSONCPP_LIBRARIES})
# set(EXTRA_LIBS ${EXTRA_LIBS} avhttp)

include_directories($ENV{OPENSSL_ROOT_DIR}/include)
link_directories($ENV{OPENSSL_ROOT_DIR}/lib)

include_directories(${CMAKE_BINARY_DIR}/inc)

file(GLOB ALL_CPP *.cpp)

add_executable(TradeBot ${ALL_CPP})
target_link_libraries(TradeBot ${EXTRA_LIBS})
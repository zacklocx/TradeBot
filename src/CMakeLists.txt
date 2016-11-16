project(TradeBot)

set(TB_VERSION_MAJOR 1)
set(TB_VERSION_MINOR 0)

set(TB_DEBUG 1)

configure_file(
	"${PROJECT_BINARY_DIR}/TradeBotConfig.h.in"
	"${PROJECT_SOURCE_DIR}/TradeBotConfig.h"
)

find_package(Boost COMPONENTS regex system thread)

set(EXTRA_LIBS ${EXTRA_LIBS} ${Boost_REGEX_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY})

add_executable(TradeBot TradeBot.cpp)
target_link_libraries(TradeBot ${EXTRA_LIBS})
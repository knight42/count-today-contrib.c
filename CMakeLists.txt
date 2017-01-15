INCLUDE (CheckIncludeFiles)

cmake_minimum_required (VERSION 2.6)
project (CountContrib)
add_executable(CountContrib main.cpp)
set_target_properties(CountContrib PROPERTIES OUTPUT_NAME "fetch")
target_link_libraries(CountContrib curl)

CHECK_INCLUDE_FILES ("curl/curl.h" HAVE_CURL_H)

install (TARGETS CountContrib DESTINATION bin)

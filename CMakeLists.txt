cmake_minimum_required(VERSION 3.1)

project(threads)

add_executable(topk_words topk_words.cpp)
set_target_properties(topk_words PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)
target_link_libraries(
		topk_words
        PRIVATE
        ${CMAKE_THREAD_LIBS_INIT}
)

add_executable(topk_words_one_thread topk_words_one_thread.cpp)
set_target_properties(topk_words_one_thread PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)
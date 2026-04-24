include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ── Shared library of non-GUI logic used by tests ─────────────────────────────
add_library(aqm_core STATIC
    ${CMAKE_SOURCE_DIR}/src/api/JsonParser.cpp
    ${CMAKE_SOURCE_DIR}/src/core/Analyzer.cpp
    ${CMAKE_SOURCE_DIR}/src/storage/JsonDatabase.cpp
    ${CMAKE_SOURCE_DIR}/src/api/RestClient.cpp
)
target_include_directories(aqm_core PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(aqm_core PUBLIC
    Qt6::Core Qt6::Network nlohmann_json::nlohmann_json)
set_target_properties(aqm_core PROPERTIES AUTOMOC ON)

# ── Helper macro ──────────────────────────────────────────────────────────────
macro(add_aqm_test name src)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE aqm_core GTest::gtest_main)
    set_target_properties(${name} PROPERTIES AUTOMOC ON)
    gtest_discover_tests(${name}
        DISCOVERY_MODE PRE_TEST
        PROPERTIES ENVIRONMENT "PATH=${Qt6_DIR}/../../../bin;$ENV{PATH}"
    )
endmacro()

include(GoogleTest)
add_aqm_test(test_JsonParser  test_JsonParser.cpp)
add_aqm_test(test_Analyzer    test_Analyzer.cpp)
add_aqm_test(test_JsonDatabase test_JsonDatabase.cpp)
# ---------------------------------------------------------------------------
# sif test suite.
#
# Every *Tests.cpp file self-registers its SIF_TEST cases (see
# TestFramework.h) - to add a new test case, add a SIF_TEST(...) block
# to an existing *Tests.cpp, or add a new *Tests.cpp file to SIF_TEST_SRC
# below; nothing else needs to change.
#
# Fixtures.h/.cpp holds shared sample data/fakes used by the tests, kept
# separate from the test logic itself.
# ---------------------------------------------------------------------------

set(SIF_TEST_SRC
    main.cpp
    TestFramework.cpp
    Fixtures.cpp
    AssetRegistryTests.cpp
    RectMathTests.cpp
    PrimitiveAnimationTests.cpp
    UIElementTests.cpp
    SpriteAnimationElementTests.cpp
    CameraTests.cpp
    RegistryNamingTests.cpp
)

add_executable(sif_tests ${SIF_TEST_SRC})
target_link_libraries(sif_tests PRIVATE sif_lib)
target_include_directories(sif_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

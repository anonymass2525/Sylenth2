cmake_minimum_required(VERSION 3.24)
project(Crystal VERSION 0.5.0 LANGUAGES C CXX)

option(SYLENTH2_BUILD_PLUGIN "Build JUCE VST3" OFF)
option(SYLENTH2_BUILD_TOOLS "Build offline renderer" ON)
option(SYLENTH2_BUILD_TESTS "Build DSP tests" ON)

add_library(sylenth2_dsp STATIC src/Synth.cpp)
target_include_directories(sylenth2_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(sylenth2_dsp PUBLIC cxx_std_17)
if(MSVC)
    target_compile_options(sylenth2_dsp PRIVATE /W4 /permissive-)
else()
    target_compile_options(sylenth2_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(SYLENTH2_BUILD_TOOLS)
    add_executable(sylenth2_render tools/RenderDemo.cpp)
    target_link_libraries(sylenth2_render PRIVATE sylenth2_dsp)
endif()

if(SYLENTH2_BUILD_TESTS)
    enable_testing()
    add_executable(sylenth2_tests tests/DspTests.cpp)
    target_link_libraries(sylenth2_tests PRIVATE sylenth2_dsp)
    add_test(NAME dsp_regression COMMAND sylenth2_tests)
endif()

if(SYLENTH2_BUILD_PLUGIN)
    include(FetchContent)
    FetchContent_Declare(juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.17-security
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(juce)
    juce_add_plugin(Crystal
        COMPANY_NAME "Independent Audio Research"
        BUNDLE_ID "com.independentaudioresearch.crystaltypegpt"
        PLUGIN_MANUFACTURER_CODE IARs
        PLUGIN_CODE Crys
        FORMATS VST3
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PRODUCT_NAME "Crystal typeGPT"
        PLUGIN_NAME "Crystal typeGPT 0.5.0")
    target_sources(Crystal PRIVATE plugin/PluginProcessor.cpp)
    target_link_libraries(Crystal PRIVATE
        sylenth2_dsp
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
    target_compile_definitions(Crystal PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)
endif()

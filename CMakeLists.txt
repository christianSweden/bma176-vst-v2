cmake_minimum_required(VERSION 3.22)
project(BM176 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(JUCE_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../JUCE" CACHE PATH "Path to JUCE source")
add_subdirectory("${JUCE_SOURCE_DIR}" JUCE)

juce_add_plugin(BM176
    COMPANY_NAME "BoMedinAudio"
    PLUGIN_MANUFACTURER_CODE BmAu
    PLUGIN_CODE bm17
    FORMATS VST3 Standalone
    PRODUCT_NAME "BM176"
    VERSION 1.0.0
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE

    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp

    Source/GUI/BM176Editor.h
    Source/GUI/BM176Editor.cpp
    Source/GUI/BM176LookAndFeel.h
    Source/GUI/BM176LookAndFeel.cpp
    Source/GUI/BM176Colours.h
    Source/GUI/BM176Geometry.h
    Source/GUI/BM176Fonts.h

    Source/Components/BMPanel.h
    Source/Components/BMPanel.cpp
    Source/Components/BMKnob.h
    Source/Components/BMKnob.cpp
    Source/Components/BMDiscreteKnob.h
    Source/Components/BMDiscreteKnob.cpp
    Source/Components/BMContinuousKnob.h
    Source/Components/BMContinuousKnob.cpp
    Source/Components/BMToggleSwitch.h
    Source/Components/BMToggleSwitch.cpp
    Source/Components/BMVerticalSlideSwitch.h
    Source/Components/BMVerticalSlideSwitch.cpp
    Source/Components/BMVU.h
    Source/Components/BMVU.cpp
    Source/Components/BMLED.h
    Source/Components/BMLED.cpp
    Source/Components/BMJackSocket.h
    Source/Components/BMJackSocket.cpp
)

target_sources(BM176 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GUI/BM176Editor.cpp
    Source/GUI/BM176LookAndFeel.cpp
    Source/Components/BMPanel.cpp
    Source/Components/BMKnob.cpp
    Source/Components/BMDiscreteKnob.cpp
    Source/Components/BMContinuousKnob.cpp
    Source/Components/BMToggleSwitch.cpp
    Source/Components/BMVerticalSlideSwitch.cpp
    Source/Components/BMVU.cpp
    Source/Components/BMLED.cpp
    Source/Components/BMJackSocket.cpp
)

target_link_libraries(BM176 PUBLIC
    juce::juce_audio_utils
    juce::juce_audio_devices
)

target_compile_definitions(BM176 PUBLIC JUCE_VST3_CAN_REPLACE_VST2=0)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(BM176 PRIVATE -Wall -Wextra -Wpedantic)
endif()

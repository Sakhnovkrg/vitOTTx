cmake_minimum_required(VERSION 3.22)

include(${CMAKE_CURRENT_SOURCE_DIR}/config.cmake)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.13 CACHE STRING "Build for 10.13")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")
set(CMAKE_CXX_STANDARD 17)

project(${PLUGIN_NAME} VERSION ${PLUGIN_VERSION})

add_subdirectory(libs/JUCE)

if(APPLE)
    set(plugin_formats VST3 AU)
elseif(UNIX)
    set(plugin_formats VST3 LV2)
else()
    set(plugin_formats VST3)
endif()

juce_add_plugin(${PLUGIN_NAME}
    COMPANY_NAME "${COMPANY_NAME}"
    PLUGIN_MANUFACTURER_CODE ${COMPANY_CODE}
    PLUGIN_CODE ${PLUGIN_CODE}
    FORMATS ${plugin_formats}
    PRODUCT_NAME "${PLUGIN_NAME}"
    BUNDLE_ID "${BUNDLE_ID}"
    LV2URI "${LV2_URI}"
    IS_SYNTH ${IS_SYNTH}
    NEEDS_MIDI_INPUT ${NEEDS_MIDI_INPUT}
    NEEDS_MIDI_OUTPUT ${NEEDS_MIDI_OUTPUT}
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "${VST3_CATEGORIES}"
)

target_sources(${PLUGIN_NAME} PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/Settings.h
    Source/Settings.cpp

    Source/ui/Theme.h
    Source/ui/HiddenCursorDrag.h
    Source/ui/HoverFader.h
    Source/ui/BandView.h
    Source/ui/BandView.cpp
    Source/ui/Knob.h
    Source/ui/Knob.cpp
    Source/ui/CrossoverHandle.h
    Source/ui/CrossoverHandle.cpp
    Source/ui/BypassButton.h
    Source/ui/BypassOverlay.h
    Source/ui/TooltipReadout.h

    Source/vital_dsp/compressor.cpp
    Source/vital_dsp/compressor.h
    Source/vital_dsp/constants.h
    Source/vital_dsp/linkwitz_riley_filter.cpp
    Source/vital_dsp/linkwitz_riley_filter.h

    Source/vital_dsp/framework/circular_queue.h
    Source/vital_dsp/framework/common.h
    Source/vital_dsp/framework/futils.h
    Source/vital_dsp/framework/matrix.h
    Source/vital_dsp/framework/note_handler.h
    Source/vital_dsp/framework/operators.cpp
    Source/vital_dsp/framework/operators.h
    Source/vital_dsp/framework/poly_utils.h
    Source/vital_dsp/framework/poly_values.h
    Source/vital_dsp/framework/processor.cpp
    Source/vital_dsp/framework/processor.h
    Source/vital_dsp/framework/synth_constants.h
    Source/vital_dsp/framework/utils.cpp
    Source/vital_dsp/framework/utils.h
    Source/vital_dsp/framework/value.cpp
    Source/vital_dsp/framework/value.h

    Source/vital_dsp/utilities/smooth_value.cpp
    Source/vital_dsp/utilities/smooth_value.h
)

target_include_directories(${PLUGIN_NAME} PRIVATE
    Source
    Source/ui
    Source/vital_dsp
)

juce_generate_juce_header(${PLUGIN_NAME})

target_compile_definitions(${PLUGIN_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(${PLUGIN_NAME}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(APPLE)
    target_compile_definitions(${PLUGIN_NAME} PUBLIC JUCE_AU=1)
endif()

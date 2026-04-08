cmake_minimum_required(VERSION 3.20)
project(pgm_jenius VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match deployment target to what Qt/libsndfile were built against
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0" CACHE STRING "")

set(CMAKE_PREFIX_PATH "/usr/local/opt/qt" ${CMAKE_PREFIX_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)
find_library(SNDFILE_LIB sndfile PATHS /usr/local/opt/libsndfile/lib REQUIRED)
set(SNDFILE_INCLUDE "/usr/local/opt/libsndfile/include")

qt_standard_project_setup()

set(SOURCES
    src/main.cpp

    # pgm
    src/pgm/ByteBuffer.cpp
    src/pgm/Parameter.cpp
    src/pgm/BaseElement.cpp
    src/pgm/Layer.cpp
    src/pgm/Pad.cpp
    src/pgm/PadEnvelope.cpp
    src/pgm/PadFilter.cpp
    src/pgm/PadMixer.cpp
    src/pgm/Slider.cpp
    src/pgm/Program.cpp
    src/pgm/MultisampleBuilder.cpp

    # audio
    src/audio/Sample.cpp
    src/audio/Markers.cpp
    src/audio/Slicer.cpp
    src/audio/AudioPlayer.cpp

    # midi
    src/midi/MidiSequenceBuilder.cpp

    # gui
    src/gui/MainWindow.cpp
    src/gui/PadButton.cpp
    src/gui/ParameterWidget.cpp
    src/gui/ProgramPanel.cpp
    src/gui/WaveformPanel.cpp
    src/gui/ChopSlicesPanel.cpp
    src/gui/MultisamplePanel.cpp
    src/gui/BatchCreatePanel.cpp
    src/gui/FolderAssignPanel.cpp
)

qt_add_executable(pgm_jenius ${SOURCES})

# App icon (macOS)
set_target_properties(pgm_jenius PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE pgm-jenius.icns
    OUTPUT_NAME "PGM-Jenius"
    MACOSX_BUNDLE_BUNDLE_NAME "PGM-Jenius"
)

# Copy icon into bundle Resources
set(ICON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../PGM-Jenius/packaging/icon128.png")
if(EXISTS "${ICON_SRC}")
    set_source_files_properties("${ICON_SRC}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(pgm_jenius PRIVATE "${ICON_SRC}")
endif()

target_include_directories(pgm_jenius PRIVATE src ${SNDFILE_INCLUDE})

# Pass repo root to main.cpp for dev-mode splash screen path
target_compile_definitions(pgm_jenius PRIVATE
    MPCMAID_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/.."
)

target_link_libraries(pgm_jenius PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    ${SNDFILE_LIB}
)

# --- CLI test binary (no Qt dependency) ---
set(PGM_SOURCES
    src/pgm/ByteBuffer.cpp
    src/pgm/Parameter.cpp
    src/pgm/BaseElement.cpp
    src/pgm/Layer.cpp
    src/pgm/Pad.cpp
    src/pgm/PadEnvelope.cpp
    src/pgm/PadFilter.cpp
    src/pgm/PadMixer.cpp
    src/pgm/Slider.cpp
    src/pgm/Program.cpp
    src/pgm/MultisampleBuilder.cpp
    src/audio/Sample.cpp
    src/audio/Markers.cpp
    src/audio/Slicer.cpp
    src/midi/MidiSequenceBuilder.cpp
)

add_executable(test_pgm test_pgm.cpp ${PGM_SOURCES})
target_include_directories(test_pgm PRIVATE src ${SNDFILE_INCLUDE})
target_compile_definitions(test_pgm PRIVATE
    MPCMAID_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/.."
)
target_link_libraries(test_pgm PRIVATE ${SNDFILE_LIB})

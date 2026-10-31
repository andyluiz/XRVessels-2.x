cmake_minimum_required(VERSION 3.19)
project(XRVessels LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This build file targets native Linux Orbiter only")
endif()

set(ORBITER_SOURCE_DIR "" CACHE PATH "Linux Orbiter source tree")
set(ORBITER_BUILD_DIR "" CACHE PATH "Configured Linux Orbiter build tree")
if(NOT EXISTS "${ORBITER_SOURCE_DIR}/Orbitersdk/include/Orbitersdk.h" OR
   NOT EXISTS "${ORBITER_BUILD_DIR}/Orbitersdk/lib/libOrbitersdk.a")
    message(FATAL_ERROR "Set ORBITER_SOURCE_DIR and ORBITER_BUILD_DIR to the Linux Orbiter source and built SDK")
endif()

set(XR_FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/framework/framework")
set(XR_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/XR1Lib")
set(XR1_DIR "${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/DeltaGliderXR1/DeltaGliderXR1")
# MSVC resolves these includes case-insensitively; Linux needs exact names.
set(XR_CASE_HEADERS "${CMAKE_CURRENT_BINARY_DIR}/case-sensitive-includes")
file(MAKE_DIRECTORY "${XR_CASE_HEADERS}")
foreach(alias IN ITEMS DeltaGliderXR1.h:deltagliderxr1.h vessel3ext.h:Vessel3Ext.h XR1Globals.h:xr1globals.h XR1AnimationPoststep.h:XR1AnimationPostStep.h XR1Colors.h:xr1colors.h orbitersdk.h:Orbitersdk.h)
    string(REPLACE ":" ";" names "${alias}")
    list(GET names 0 requested)
    list(GET names 1 actual)
    file(WRITE "${XR_CASE_HEADERS}/${requested}" "#include \"${actual}\"\n")
endforeach()

include(CTest)
if(BUILD_TESTING)
    add_executable(xr_window_position_test tests/XRWindowPositionFileTest.cpp)
    target_compile_features(xr_window_position_test PRIVATE cxx_std_17)
    target_include_directories(xr_window_position_test PRIVATE "${XR_COMMON_DIR}")
    target_link_libraries(xr_window_position_test PRIVATE Qt6::Core)
    add_test(NAME XRWindowPositionFile COMMAND xr_window_position_test)

    add_executable(xr_textbox_test tests/XRTextBoxTest.cpp "${XR_COMMON_DIR}/TextBox.cpp")
    target_compile_features(xr_textbox_test PRIVATE cxx_std_17)
    target_include_directories(xr_textbox_test PRIVATE
        "${XR_COMMON_DIR}" "${XR_FRAMEWORK_DIR}" "${XR_CASE_HEADERS}" "${ORBITER_SOURCE_DIR}/Orbitersdk/include")
    target_link_libraries(xr_textbox_test PRIVATE Qt6::Gui)
    add_test(NAME XRTextBox COMMAND xr_textbox_test)
    set_tests_properties(XRTextBox PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(xr_render_window_test tests/XRRenderWindowTest.cpp)
    target_compile_features(xr_render_window_test PRIVATE cxx_std_17)
    target_include_directories(xr_render_window_test PRIVATE "${XR_FRAMEWORK_DIR}")
    target_link_libraries(xr_render_window_test PRIVATE Qt6::Gui)
    add_test(NAME XRRenderWindow COMMAND xr_render_window_test)
    set_tests_properties(XRRenderWindow PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_library(xr_flagged_fixture MODULE tests/XRModuleFlagFixture.cpp)
    target_compile_features(xr_flagged_fixture PRIVATE cxx_std_17)
    target_compile_definitions(xr_flagged_fixture PRIVATE XR_FLAGGED_FIXTURE)
    set_target_properties(xr_flagged_fixture PROPERTIES PREFIX "" CXX_VISIBILITY_PRESET hidden)
    target_compile_options(xr_flagged_fixture PRIVATE -fno-rtti)
    add_library(xr_unflagged_fixture MODULE tests/XRModuleFlagFixture.cpp)
    target_compile_features(xr_unflagged_fixture PRIVATE cxx_std_17)
    set_target_properties(xr_unflagged_fixture PROPERTIES PREFIX "" CXX_VISIBILITY_PRESET hidden)
    target_compile_options(xr_unflagged_fixture PRIVATE -fno-rtti)
    add_library(xr_disabled_fixture MODULE tests/XRModuleFlagFixture.cpp)
    target_compile_features(xr_disabled_fixture PRIVATE cxx_std_17)
    target_compile_definitions(xr_disabled_fixture PRIVATE XR_DISABLED_FIXTURE)
    set_target_properties(xr_disabled_fixture PROPERTIES PREFIX "" CXX_VISIBILITY_PRESET hidden)
    target_compile_options(xr_disabled_fixture PRIVATE -fno-rtti)

    add_executable(xr_module_flag_test tests/XRModuleFlagTest.cpp)
    target_compile_features(xr_module_flag_test PRIVATE cxx_std_17)
    target_include_directories(xr_module_flag_test PRIVATE "${XR_FRAMEWORK_DIR}")
    target_compile_definitions(xr_module_flag_test PRIVATE
        XR_FLAGGED_LIBRARY="$<TARGET_FILE:xr_flagged_fixture>"
        XR_UNFLAGGED_LIBRARY="$<TARGET_FILE:xr_unflagged_fixture>"
        XR_DISABLED_LIBRARY="$<TARGET_FILE:xr_disabled_fixture>")
    set_target_properties(xr_module_flag_test PROPERTIES ENABLE_EXPORTS ON)
    target_link_libraries(xr_module_flag_test PRIVATE ${CMAKE_DL_LIBS})
    add_dependencies(xr_module_flag_test xr_flagged_fixture xr_unflagged_fixture xr_disabled_fixture)
    add_test(NAME XRModuleFlag COMMAND xr_module_flag_test)

    add_executable(xr_control_dialog_test tests/XRControlDialogTest.cpp)
    target_compile_features(xr_control_dialog_test PRIVATE cxx_std_17)
    target_include_directories(xr_control_dialog_test PRIVATE "${XR_FRAMEWORK_DIR}")
    target_link_libraries(xr_control_dialog_test PRIVATE Qt6::Widgets)
    add_test(NAME XRControlDialog COMMAND xr_control_dialog_test)
    set_tests_properties(XRControlDialog PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(xr_qt_drawing_test tests/XRQtDrawingTest.cpp)
    target_compile_features(xr_qt_drawing_test PRIVATE cxx_std_17)
    target_include_directories(xr_qt_drawing_test PRIVATE "${XR_FRAMEWORK_DIR}" "${ORBITER_SOURCE_DIR}/Orbitersdk/include")
    target_link_libraries(xr_qt_drawing_test PRIVATE Qt6::Gui)
    add_test(NAME XRQtDrawing COMMAND xr_qt_drawing_test)
    set_tests_properties(XRQtDrawing PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(xr_template_cleanup_test tests/XRTemplateCleanupTest.cpp)
    target_compile_features(xr_template_cleanup_test PRIVATE cxx_std_17)
    target_include_directories(xr_template_cleanup_test PRIVATE "${XR_FRAMEWORK_DIR}")
    add_test(NAME XRTemplateCleanup COMMAND xr_template_cleanup_test)
endif()
file(GLOB XR_FRAMEWORK_SOURCES CONFIGURE_DEPENDS "${XR_FRAMEWORK_DIR}/*.cpp")
list(REMOVE_ITEM XR_FRAMEWORK_SOURCES "${XR_FRAMEWORK_DIR}/RegKeyManager.cpp")
file(GLOB XR_COMMON_SOURCES CONFIGURE_DEPENDS "${XR_COMMON_DIR}/*.cpp")
set(ORBITER_SOURCE_SDK_INCLUDE_DIR "${ORBITER_SOURCE_DIR}/Orbitersdk/include")
include("${ORBITER_SOURCE_DIR}/cmake/rcresources.cmake")

foreach(vessel IN ITEMS XR2Ravenstar XR5Vanguard)
    set(vessel_dir "${CMAKE_CURRENT_SOURCE_DIR}/XRVessels/${vessel}/${vessel}")
    file(GLOB vessel_sources CONFIGURE_DEPENDS "${vessel_dir}/*.cpp")
    add_library(${vessel} MODULE ${vessel_sources} ${XR_COMMON_SOURCES} ${XR_FRAMEWORK_SOURCES})
    orbiter_rc_resources(${vessel} "${vessel_dir}/${vessel}.rc")
    target_compile_features(${vessel} PRIVATE cxx_std_17)
    target_include_directories(${vessel} PRIVATE
        "${vessel_dir}"
        "${XR_COMMON_DIR}"
        "${XR_CASE_HEADERS}"
        "${XR1_DIR}"
        "${XR_FRAMEWORK_DIR}"
        "${ORBITER_SOURCE_DIR}/Orbitersdk/include"
        "${ORBITER_BUILD_DIR}/Orbitersdk/include"
        "${ORBITER_BUILD_DIR}/Orbitersdk/XRSound"
        "${ORBITER_SOURCE_DIR}/Src/Orbiter"
        "${ORBITER_SOURCE_DIR}/Src/Plugin/ScnEditor"
        "${ORBITER_SOURCE_DIR}/Sound/XRSound/src"
    )
    target_link_libraries(${vessel} PRIVATE
        "${ORBITER_BUILD_DIR}/Orbitersdk/lib/libOrbitersdk.a"
        "${ORBITER_BUILD_DIR}/Orbitersdk/lib/libDlgCtrl.a"
        "${ORBITER_BUILD_DIR}/Orbitersdk/XRSound/libXRSound.a"
        Qt6::Widgets
        ${CMAKE_DL_LIBS}
    )
    set_target_properties(${vessel} PROPERTIES PREFIX "" LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Modules" BUILD_RPATH "$ORIGIN;$ORIGIN/..")
endforeach()

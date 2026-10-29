cmake_minimum_required(VERSION 3.20)
project(FitFlexFigure LANGUAGES CXX)

add_executable(FitFlexFigure WIN32
    main.cpp
    ArticulatedExerciseControl.cpp
    ArticulatedExerciseControl.h
)

target_compile_features(FitFlexFigure PRIVATE cxx_std_17)
target_compile_definitions(FitFlexFigure PRIVATE UNICODE _UNICODE NOMINMAX)
target_link_libraries(FitFlexFigure PRIVATE gdiplus user32 gdi32)

if(MSVC)
    target_compile_options(FitFlexFigure PRIVATE /W4 /utf-8)
endif()

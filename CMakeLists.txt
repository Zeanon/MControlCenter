cmake_minimum_required(VERSION 3.25)

project(MControlCenter VERSION 0.5.3 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS
    Widgets
    LinguistTools
    DBus
    Network
)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.ui
    src/mainwindow.cpp src/mainwindow.h
    src/operate.cpp src/operate.h
    src/powermonitor.cpp src/powermonitor.h
    src/helper.cpp src/helper.h
    src/settings.cpp src/settings.h
    src/resources.qrc
)

#TODO: use I18N_SOURCE_LANGUAGE in qt_standard_project_setup() for translation
#when buntu/mint ship qt6.8
set(TS_FILES
    translations/MControlCenter_de_DE.ts
    translations/MControlCenter_en.ts
    translations/MControlCenter_es.ts
    translations/MControlCenter_eu.ts
    translations/MControlCenter_fi_FI.ts
    translations/MControlCenter_fr.ts
    translations/MControlCenter_hu.ts
    translations/MControlCenter_it.ts
    translations/MControlCenter_nb.ts
    translations/MControlCenter_nl_NL.ts
    translations/MControlCenter_pt_BR.ts
    translations/MControlCenter_pt_PT.ts
    translations/MControlCenter_ru.ts
    translations/MControlCenter_tr.ts
    translations/MControlCenter_vi.ts
    translations/MControlCenter_zh_CN.ts
)

qt_add_executable(mcontrolcenter
    ${PROJECT_SOURCES}
)

qt_add_translations(mcontrolcenter
    TS_FILES ${TS_FILES}
    LUPDATE_OPTIONS
        -no-obsolete
        -locations none
)

add_subdirectory(src/helper ${CMAKE_BINARY_DIR}/helper)

target_link_libraries(mcontrolcenter PRIVATE
    Qt6::Widgets
    Qt6::DBus
    Qt6::Network
)

target_compile_definitions(mcontrolcenter PRIVATE
    MControlCenter_VERSION="${PROJECT_VERSION}")

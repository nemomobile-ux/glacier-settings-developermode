SET(SRC developmentsettingsplugin.cpp)
SET(HEADERS developmentsettingsplugin.h)

SET(PLUGINS_FILES
    development.qml
    development.svg)

set(CMAKE_AUTOMOC ON)

add_definitions(-DQT_PLUGIN)

add_library(development MODULE ${SRC} ${HEADERS})
target_include_directories(development PUBLIC "/usr/include/glacier-settings/")

target_link_libraries(development
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    -lglaciersettings)

install(TARGETS development
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/glacier-settings/)

INSTALL(FILES ${PLUGINS_FILES}
    DESTINATION /usr/share/glacier-settings/plugins/development/)

# Translations
file(GLOB TS_FILES translations/*.ts)
#set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${CMAKE_INSTALL_LOCALEDIR})
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(development_translations DEPENDS ${QM_FILES})
add_dependencies(development development_translations)

install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/glacier-settings/translations)

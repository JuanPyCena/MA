

install(DIRECTORY bbtests DESTINATION avauth/ USE_SOURCE_PERMISSIONS COMPONENT main)

add_subdirectory(avauthlib)
add_subdirectory(avauthtestcli)

add_subdirectory(avauthtestgui)

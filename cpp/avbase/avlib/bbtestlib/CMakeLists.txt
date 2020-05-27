
av_install_subdirectory(. COMPONENT main PATTERN "CMakeLists.txt" EXCLUDE)

environment_script_add_source_subdirectory_to_environment_variable(PYTHONPATH .)

#! eof

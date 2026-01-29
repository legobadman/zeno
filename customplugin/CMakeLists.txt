add_library(customplugin SHARED
    customplugin.cpp
)

target_include_directories(customplugin PUBLIC "${PROJECT_SOURCE_DIR}/interface")
add_llvm_library(ISPRE MODULE
  ISPRE.cpp
  ISPRE2.cpp
  ISPRE3.cpp
  ISPRE4.cpp
  # Include any additional .cpp files in this directory with passes you want included
  PLUGIN_TOOL
  opt
)

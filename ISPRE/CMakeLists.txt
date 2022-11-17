add_llvm_library(ISPRE MODULE
  ISPRE.cpp
  # Include any additional .cpp files in this directory with passes you want included
  PLUGIN_TOOL
  opt
)

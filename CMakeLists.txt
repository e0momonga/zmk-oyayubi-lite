if(CONFIG_ZMK_BEHAVIOR_OYAYUBI)
  target_sources(app PRIVATE src/behaviors/behavior_oyayubi.c)
endif()
zephyr_include_directories(include)

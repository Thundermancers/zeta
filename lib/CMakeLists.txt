# Generating properties macro create
add_custom_target(
  generateFiles ALL
  COMMAND python3 ${PROJECT_SOURCE_DIR}/mount_system.py ${CMAKE_CURRENT_SOURCE_DIR}
  BYPRODUCTS "${PROJECT_BINARY_DIR}/zephyr/src/generated/pdb.c" "${PROJECT_BINARY_DIR}/zephyr/src/generated/pdb_threads.c"
  COMMENT "Generating files from templates"
  )
add_dependencies(app generateFiles)

zephyr_sources_ifdef(CONFIG_PDB ${PROJECT_BINARY_DIR}/zephyr/src/generated/pdb.c)
zephyr_sources_ifdef(CONFIG_PDB ${PROJECT_BINARY_DIR}/zephyr/src/generated/pdb_threads.c)

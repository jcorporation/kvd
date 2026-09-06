# SPDX-License-Identifier: GPL-3.0-or-later
# (c) 2026 Juergen Mang <mail@jcgames.de>
# https://github.com/jcorporation/kvd

add_executable(kvd "")

if(CMAKE_BUILD_TYPE MATCHES "(Release|Debug)")
  target_compile_options(kvd
    PRIVATE
      "-Wconversion"
      "-Wformat=2"
  )
endif()

target_include_directories(kvd
  PRIVATE 
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}
)

target_sources(kvd
  PRIVATE
    lib/config.c
    lib/filehandler.c
    lib/global_data.c
    lib/log.c
    lib/mem.c
    lib/mpack.c
    lib/options.c
    lib/signal.c
    kvd_store.c
    main.c
    rest.c
    server.c
)

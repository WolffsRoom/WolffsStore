cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED ENV{VITASDK})
  message(FATAL_ERROR "VITASDK not set")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "Vita toolchain")
project(WolffsStore C)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall -Wno-format-truncation")

set(WS_TITLEID "WOLF00001")
set(WS_UPDATER_TITLEID "WOLF00002")
# LiveArea padrão: assets/livearea (icon0 128x128, bg0 840x500, startup 280x158, PNG indexado de 8 bits)
set(WS_SFO_VERSION "00.91")
set(WS_FILE_VERSION "0.91")
set(WS_VPK_NAME "WolffsStore.vpk")
set(WS_ARTIFACTS_DIR "${CMAKE_SOURCE_DIR}/artifacts/current")

# Embute um arquivo binário como objeto ELF (ld -b binary), gerando os símbolos
# _binary_<caminho>_start/_end. Mesmo esquema do VitaSeed/VitaShell.
function(add_resources out_var)
  set(result)
  foreach(ref_f ${ARGN})
    set(out_f "${CMAKE_CURRENT_BINARY_DIR}/${ref_f}.o")
    get_filename_component(out_dir ${out_f} DIRECTORY)
    add_custom_command(OUTPUT ${out_f}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
      COMMAND ${CMAKE_LINKER} -r -b binary -o ${out_f} ${ref_f}
      DEPENDS ${ref_f}
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      COMMENT "Embutindo ${ref_f}"
      VERBATIM)
    list(APPEND result ${out_f})
  endforeach()
  set(${out_var} "${result}" PARENT_SCOPE)
endfunction()

add_resources(ws_res resources/head.bin)

# ------------------------------------------------------------------ app principal

add_executable(${PROJECT_NAME}
  ${ws_res}
  src/main.c
  src/gfx.c
  src/audio.c
  src/catalog.c
  src/store.c
  src/sync.c
  src/jobs.c
  src/net.c
  src/install.c
  src/fsutil.c
  src/registry.c
  src/json.c
  src/video.c
  src/vendor/sha1.c
)

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_compile_definitions(${PROJECT_NAME} PRIVATE WS_VERSION="${WS_FILE_VERSION}")
# mesmas bibliotecas de rede/zip do VitaSeed (curl + OpenSSL + libarchive)
target_link_libraries(${PROJECT_NAME}
  vita2d
  freetype
  png
  jpeg
  curl
  ssl
  crypto
  pthread
  archive
  zstd
  lzma
  expat
  bz2
  z
  SceGxm_stub
  SceDisplay_stub
  SceCommonDialog_stub
  SceSysmodule_stub
  SceCtrl_stub
  SceTouch_stub
  SceIme_stub
  SceAudio_stub
  SceAvPlayer_stub
  SceAppMgr_stub
  SceAppUtil_stub
  SceIofilemgr_stub
  SceNet_stub
  SceNetCtl_stub
  SceRtc_stub
  SceShellSvc_stub
  ScePromoterUtil_stub
  SceSysmem_stub
  SceKernelThreadMgr_stub
  SceKernelModulemgr_stub
  SceProcessmgr_stub
  SceLibKernel_stub
  m
  mathneon
  c
)

# UNSAFE: o promoter (instalar VPKs) precisa de permissões estendidas, como no VitaShell/VitaSeed
vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)

# ------------------------------------------------------------------ updater (WOLF00002)

add_executable(WolffsUpdater updater/main.c)
target_link_libraries(WolffsUpdater
  vita2d
  SceGxm_stub
  SceDisplay_stub
  SceCommonDialog_stub
  SceSysmodule_stub
  SceAppMgr_stub
  ScePromoterUtil_stub
  SceKernelThreadMgr_stub
  SceProcessmgr_stub
  SceLibKernel_stub
  m
  c
)
vita_create_self(updater_eboot.bin WolffsUpdater UNSAFE)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/updater_param.sfo
  COMMAND vita-mksfoex -s TITLE_ID=${WS_UPDATER_TITLEID} -s APP_VER=${WS_SFO_VERSION}
          "Wolff's Store Updater" ${CMAKE_CURRENT_BINARY_DIR}/updater_param.sfo
  VERBATIM)
add_custom_target(ws_updater_sfo ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/updater_param.sfo)

# ------------------------------------------------------------------ VPK

# O catálogo textual garante uma tela útil sem rede e data/tex contém os
# recursos da interface. Capas e capturas de data/posts não entram no VPK:
# são baixadas do site para o cache na primeira sincronização.
file(GLOB_RECURSE ws_data RELATIVE ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/data/tex/*
  ${CMAKE_SOURCE_DIR}/data/catalog.txt)
set(ws_files)
foreach(f ${ws_data})
  list(APPEND ws_files ${CMAKE_SOURCE_DIR}/${f} ${f})
endforeach()

vita_create_vpk(${WS_VPK_NAME} ${WS_TITLEID} eboot.bin
  VERSION ${WS_SFO_VERSION}
  NAME "Wolff's Store"
  FILE
    ${CMAKE_SOURCE_DIR}/assets/livearea/icon0.png sce_sys/icon0.png
    ${CMAKE_SOURCE_DIR}/assets/livearea/bg0.png sce_sys/livearea/contents/bg0.png
    ${CMAKE_SOURCE_DIR}/assets/livearea/startup.png sce_sys/livearea/contents/startup.png
    ${CMAKE_SOURCE_DIR}/assets/livearea/template.xml sce_sys/livearea/contents/template.xml
    ${CMAKE_CURRENT_BINARY_DIR}/updater_eboot.bin updater/eboot.bin
    ${CMAKE_CURRENT_BINARY_DIR}/updater_param.sfo updater/sce_sys/param.sfo
    ${CMAKE_SOURCE_DIR}/assets/livearea/icon0.png updater/sce_sys/icon0.png
    ${CMAKE_SOURCE_DIR}/data/tex/paw4.wrt updater/data/tex/paw4.wrt
    ${CMAKE_SOURCE_DIR}/data/tex/font_title.wrt updater/data/tex/font_title.wrt
    ${ws_files}
)
add_dependencies(${WS_VPK_NAME}-vpk updater_eboot.bin-self ws_updater_sfo)

add_custom_target(ws_copy_artifact ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory ${WS_ARTIFACTS_DIR}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${WS_VPK_NAME} ${WS_ARTIFACTS_DIR}/${WS_VPK_NAME}
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${WS_VPK_NAME}
  COMMENT "Copiando ${WS_VPK_NAME} para artifacts/current"
)

# This file has no copyright assigned and is placed in the public domain.
# This file is the build script of the libresolw compatibility library:
#   https://github.com/treeswift/libresolw
# No warranty is given; refer to the LICENSE file in the project root.

# Inspired by: https://github.com/krux02/minimal_cmake_example (public domain)

cmake_minimum_required(VERSION 3.2)
include(CheckIncludeFileCXX REQUIRED)
project(resolw)

set(libapiheaders
"include/resolw/resolw_types.h"
"include/resolv.h"
)

set(libdepheaders "")

set(libsources
"src/dns.h"
"src/dns.cpp" # TODO
"src/err.cpp"
"src/ndb.cpp" # TODO
"src/rnd.cpp"
)

option(USE_BSD_SOURCE "Use BSD-originated source files. ON=3-clause BSD license, OFF=public domain" ON)
option(INSTALL_H_FOR_ALL "Install compat *.h directly to ${prefix}/include. OFF=${prefix}/include/resolw" ON)

if(${INSTALL_H_FOR_ALL})
    set(compat_path "include")
else()
    set(compat_path "include/resolw")
endif()

if(${USE_BSD_SOURCE})
    set(XXBSD "xxbsd")
    set(compiledefs ${compiledefs} "__BSD_VISIBLE=1")
    set(libsources ${libsources} "src/xxbsd/res_comp.c") # dn_{comp|expand|skipname}
else()
    set(XXBSD "adhoc")
endif()

set(compat_dirs "") # -isystem include paths for compatibility headers

macro(install_missing_header project_path project_file)
    set(complete_path "${project_path}/${project_file}")
    get_filename_component(install_path ${project_file} DIRECTORY)
    install(FILES ${complete_path} DESTINATION "${compat_path}/${install_path}")
    set(libdepheaders ${libdepheaders} ${complete_path})
    set(compat_dirs ${compat_dirs} ${project_path})
endmacro()

macro(catchup_missing_header project_path project_file)
    # some of the header files might already be provided:
    CHECK_INCLUDE_FILE_CXX(${project_file} HAS_INCLUDE_H)
    if(NOT HAS_INCLUDE_H)
        install_missing_header(${project_path} ${project_file})
    endif()
endmacro()


# "adhoc" => we believe that our ad-hoc implementation is good enough;
# ${XXBSD} => we allow choice between public domain and BSD-originated
catchup_missing_header("include/resolw/adhoc/netdb_h" "netdb.h") # getrrsetbyname, freerrset
catchup_missing_header("include/resolw/${XXBSD}/endian_h" "sys/endian.h") # system endianness
catchup_missing_header("include/resolw/${XXBSD}/nameser_h" "arpa/nameser.h") # common Internet service names

# Forwarding headers are one-liners too trivial to be copyrightable.
# As of release 0.0.1 they are (exact contents given below):
# echo '#include <winsock2.h>' > "arpa/inet.h"
# echo '#include <winsock2.h>' > "netinet/{in,in_systm,ip}.h"
# echo '#include <ws2tcpip.h>' > "sys/socket.h"
set(ws2_fwd "include/resolw/ws2_fwd")
catchup_missing_header(${ws2_fwd} "sys/socket.h")
catchup_missing_header(${ws2_fwd} "arpa/inet.h")
catchup_missing_header(${ws2_fwd} "netinet/in.h")
catchup_missing_header(${ws2_fwd} "netinet/in_systm.h")
catchup_missing_header(${ws2_fwd} "netinet/ip.h")

list(REMOVE_DUPLICATES compat_dirs)
set(libheaders ${libapiheaders} ${libdepheaders})

if(MINGW)
    # only affects the library & samples, not headers
    set(compiledefs ${compiledefs} "ERRNO_IS_LVALUE")
endif()

if(MSVC)
    set(compile_flags "/Wall")
else()
    set(compile_flags "-fPIC" "-Wall")
endif()

include_directories("include")

add_library(resolw ${libheaders} ${libsources})
target_compile_options(resolw PRIVATE ${compile_flags})
target_compile_definitions(resolw PRIVATE ${compiledefs})
target_include_directories(resolw PRIVATE ${compat_dirs})
target_link_libraries(resolw -lws2_32 -ldnsapi -lkernel32 -lntdll)
#  -ladvapi32 -lsecur32

set(exesources "samples/namequery.cpp")
add_executable(namequery ${exesources})
target_compile_definitions(namequery PRIVATE ${compiledefs})
target_include_directories(namequery PRIVATE ${compat_dirs})
target_link_libraries(namequery resolw)

install(FILES "include/resolw/resolw_types.h"
                                 DESTINATION include/resolw)
install(FILES "include/resolv.h" DESTINATION include)
install(TARGETS resolw namequery DESTINATION bin)

set(CPACK_PACKAGE_NAME "resolw")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_MONOLITHIC_INSTALL 1)
include(CPack)

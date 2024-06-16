#  Copyright Matus Chochlik.
#  Distributed under the Boost Software License, Version 1.0.
#  See accompanying file LICENSE_1_0.txt or copy at
#  https://www.boost.org/LICENSE_1_0.txt
#
# Package specific options
#  Debian
#   Dependencies
set(CXX_RUNTIME_PKGS "libc6,libc++1-17")
set(CPACK_DEBIAN_APP-TOOLS_PACKAGE_DEPENDS "python3,python3-pip")
set(CPACK_DEBIAN_APP-EXAMPLES_PACKAGE_DEPENDS "${CXX_RUNTIME_PKGS},libsystemd0,zlib1g,libssl3,libzip4,libglfw3,libglew2.2,libopenal1,libalut0")
set(CPACK_DEBIAN_APP-MODEL-VIEWER_PACKAGE_DEPENDS "${CXX_RUNTIME_PKGS},libsystemd0,zlib1g,libssl3,libzip4,libglfw3,libglew2.2,libopenal1,libalut0")
set(CPACK_DEBIAN_APP-SKY-VIEWER_PACKAGE_DEPENDS "${CXX_RUNTIME_PKGS},libsystemd0,zlib1g,libssl3,libzip4,libglfw3,libglew2.2,libopenal1,libalut0")
set(CPACK_DEBIAN_APP-TILING-VIEWER_PACKAGE_DEPENDS "${CXX_RUNTIME_PKGS},libsystemd0,zlib1g,libssl3,libzip4,libglfw3,libglew2.2,libopenal1,libalut0")
set(CPACK_DEBIAN_APP-RESOURCE-PROVIDER_PACKAGE_DEPENDS "${CXX_RUNTIME_PKGS},libsystemd0,zlib1g,libssl3,libzip4,libglfw3,libglew2.2,libopenal1,libalut0")
#   Descriptions
set(CPACK_DEBIAN_APP-TOOLS_DESCRIPTION "Collection of command-line utilities useful with EAGine applications.")
set(CPACK_DEBIAN_APP-TOOLS_DESCRIPTION "EAGine App examples.")
set(CPACK_DEBIAN_APP-SKY-VIEWER_DESCRIPTION "Application for viewing sky-box textures.")
set(CPACK_DEBIAN_APP-MODEL-VIEWER_DESCRIPTION "Application for viewing 3D models.")
set(CPACK_DEBIAN_APP-TILING-VIEWER_DESCRIPTION "Application for viewing tiling and tileset textures.")
set(CPACK_DEBIAN_APP-RESOURCE-PROVIDER_DESCRIPTION "Resource provider message bus server.")

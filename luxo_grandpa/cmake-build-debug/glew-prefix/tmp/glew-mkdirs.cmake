# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/src/glew"
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/src/glew-build"
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix"
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/tmp"
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/src/glew-stamp"
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/src"
  "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/src/glew-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/src/glew-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/User/Documents/GitHub/grafika_hf2/luxo_grandpa/cmake-build-debug/glew-prefix/src/glew-stamp${cfgdir}") # cfgdir has leading slash
endif()

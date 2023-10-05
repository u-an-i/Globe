# Globe
 raster tiles maps on sphere renderer  
  
# libraries utilised
SDL, libjpeg-turbo, c-hashmap  
  
- https://github.com/libsdl-org/SDL/tree/SDL2 (clone and follow docs/README-...)
- https://libjpeg-turbo.org/ (download and install https://sourceforge.net/projects/libjpeg-turbo/files/ 3.0.0 and reference lib for linker)
- https://github.com/Mashpoe/c-hashmap with https://github.com/Mashpoe/c-hashmap/pull/6 (copy files to this project)
  
(versions as of writing these lines)  
  
# Windows application
distribution compiled in Visual Studio 2022 17.7.4 with Intel C++ Compiler 2023
see https://www.intel.com/content/www/us/en/docs/cpp-compiler/developer-guide-reference/2021-10/use-the-intel-c-compiler-classic-math-library.html
for compiler options regarding long double usage
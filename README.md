# Globe
ray casting renderer orthogonally projecting a sphere textured in a raster tiles map  
  
# libraries utilised
SDL, libjpeg-turbo, c-hashmap, miniLZO  
  
- https://github.com/libsdl-org/SDL/tree/SDL2  
  (clone and follow docs/README-...)
- https://libjpeg-turbo.org/  
  (download and install  
  https://sourceforge.net/projects/libjpeg-turbo/files/  
  3.0.0 and reference lib for linker)
- https://github.com/Mashpoe/c-hashmap  
  with https://github.com/Mashpoe/c-hashmap/pull/6  
  (copy files to this project)  
  (compiled as DLL in distributables,  
  DLL source header named "c-hashmap.h")
- http://www.oberhumer.com/opensource/lzo/  
  in form of miniLZO  
  (copy files to this project)  
  (compiled as DLL in distributables,  
  DLL source header named "miniLZO.h")
  
(versions as of writing these lines)  
  
# Windows application
distributables built  
  in  
Visual Studio 2022 17.7.4  
  with  
Intel C++ Compiler 2023.2.1  
  
see  
https://www.intel.com/content/www/us/en/docs/cpp-compiler/developer-guide-reference/2021-10/use-the-intel-c-compiler-classic-math-library.html  
for compiler options regarding long double utilisation

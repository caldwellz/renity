# Renity

An experiment in C++ game engine design. It aims to support both 2D and lightweight 3D rendering via OpenGL ES 3.0, but with enough abstraction to enable a possible future Vulkan backend.

### Requirements

Renity uses the Meson build system and, at the moment, depends on an external Python3 (+ wheezy.template), SDL3, SDL3_image, PhysFS, and optionally Git and Doxygen.
It also uses some in-tree 3rd-party code (Dear ImGui, for one) embedded as Git submodule(s), so it is strongly recommended to clone the source recursively using Git.
On Debian-based systems (including Ubuntu), usually this means you just need to have a C++ compiler of your choice installed (clang or g++) and then run:
```
sudo apt install git python3 ninja-build doxygen libsdl3-dev libsdl3-image-dev libphysfs-dev
pip3 install meson wheezy.template
git clone --recurse-submodules https://github.com/caldwellz/renity.git
cd renity
meson mybuild
ninja -C mybuild install
```
Engine documentation is not automatically installed, but if generated as part of the build, it can be found at docs/html/index.htm - this README.md is the main page.

### Coding Standards

This project tries to follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) and should be formatted according to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) as closely as possible, with the exception of class filenames (CamelCase) and function names (camelCase). However, chances are you may find occasional code that doesn't quite meet these standards. If so, please check whether it's been reported as a bug and report it if not, or better yet, just send in a patch!

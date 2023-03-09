# Renity

An experiment in 2D C++ game engine design, leveraging SDL for rendering and cross-platform compatibility.

### Requirements

Renity uses the Meson build system and, at the moment, depends on SDL3, SDL3_image, PhysFS, and optionally Git and Doxygen.
Since it uses some in-tree 3rd-party code (Dear ImGui, for one) embedded as Git submodule(s), it is strongly recommended to clone the source recursively using Git.
On Debian-based systems (including Ubuntu), usually this means you just need to have a C++ compiler of your choice installed (clang or g++) and then run:
```
sudo apt install git meson ninja-build doxygen libsdl3-dev libsdl3-image-dev libphysfs-dev
git clone --recurse-submodules https://github.com/caldwellz/renity.git
cd renity
meson mybuild
cd mybuild
ninja install
```
Engine documentation is not automatically installed, but if generated as part of the build, it can be found at docs/html/index.htm - this README.md is the main page.

### Coding Standards

This project tries to follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) and should be formatted according to the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) as closely as possible, with the exception of class filenames (CamelCase) and function names (camelCase). However, chances are you may find occasional code that doesn't quite meet these standards. If so, please check whether it's been reported as a bug and report it if not, or better yet, just send in a patch!

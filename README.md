# Renity

An experiment in 2D C++ game engine design, leveraging SDL for rendering and cross-platform compatibility.

### Requirements

At the moment, Renity uses the Meson build system and depends on SDL2, SDL2_image, and PhysFS.  
On Debian-based systems (including Ubuntu), usually this means you just need to have a C++ compiler of your choice installed (clang or g++) and then from the project directory, run:
```
sudo apt install meson libsdl2-dev libsdl2-image-dev libphysfs-dev
meson builddir
cd builddir
ninja install
```

### Coding Standards

This project tries to follow the [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines) and [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) as closely as possible, with the exception of indentation (4 spaces), class filenames (CamelCase) and function names (camelCase). However, chances are you may find occasional code that doesn't quite meet these specifications. If so, please check whether it's been reported as a bug and report it if not, or better yet, just send in a patch!

# Tests list
tests = [
    ['surface_utils', '.c']
  , ['texture_utils', '.c']
  , ['version', '.c']
  , ['Dimension2D', '.cc']
  , ['Point2D', '.cc']
  , ['Rect2D', '.cc']
  , ['Texture', '.cc']
  , ['Window', '.cc']
]


# Test dependencies
# - SDL2_main
dep_sdlmain = dependency('sdl2main', required : false)
if not dep_sdlmain.found()
    dep_sdlmain = cppcomp.find_library('SDL2main')
endif

test_deps = lib_deps + [dep_sdlmain]

foreach t : tests
    exe = executable(t[0], t[0] + t[1], dependencies : test_deps, link_with : lib_target, include_directories : lib_incdir)
    test(t[0], exe)
endforeach

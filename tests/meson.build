# Tests list
tests = [
    ['surface_utils', '.c']
  , ['texture_utils', '.c']
  , ['version', '.c']
  , ['Dictionary', '.cc']
  , ['Dimension2D', '.cc']
  , ['Point2D', '.cc']
  , ['Rect2D', '.cc']
#  , ['Sprite', '.cc']
#  , ['Texture', '.cc']
  , ['Window', '.cc']
]

test_deps = lib_deps + []

foreach t : tests
    exe = executable(
      t[0], t[0] + t[1]
    , dependencies: test_deps
    , link_with: lib_target
    , include_directories: lib_incdirs
    , win_subsystem: 'console')
    test(t[0], exe)
endforeach

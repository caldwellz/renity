# Generate HTML documentation of the engine if Doxygen is found
doxygen = find_program('doxygen', required : false)
if doxygen.found()
  doxyfile = configure_file(configuration : version_data
    , input : 'Doxyfile.in'
    , output : 'Doxyfile')
  run_command(doxygen, doxyfile, capture : false, check : false)
endif

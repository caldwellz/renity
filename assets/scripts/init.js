'use strict';
const lib = require('/assets/scripts/lib.js');

function init() {
  // TODO: Register actual action handlers (i.e. event listeners) and do other one-time init tasks here...
  Actions.assignCategory('ExampleAction', 'ScriptCategory');
  Actions.assignCategory('ExampleAction2', 'ScriptCategory');
  Actions.subscribe('ScriptCategory', lib.handleExampleActionCategory);
  Actions.post('ExampleAction', ['foo', 2, -65536]);
  Actions.post(Helpers.getId('ExampleAction2'), [6.66, true, null]);

  // Return false to indicate application initialization failed, if needed
  return true;
}
init();

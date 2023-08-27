'use strict';
const lib = require('/assets/scripts/lib.js');

function init() {
  // TODO: Register event listeners / action handlers and do other one-time init tasks here...
  console.log({ foo: lib.sayHello() });

  // Return false to indicate application initialization failed, if needed
  return true;
}
init();

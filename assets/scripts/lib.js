'use strict';

module.exports = {
  // All export definitions should happen in here so they don't pollute the global scope
  sayHello: function () {
    console.log('Hello world!');
    return 'bar';
  }
}

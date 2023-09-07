'use strict';

module.exports = {
  // All export definitions should happen here so they don't pollute the global scope
  sayHello: function () {
    console.log('Hello world!');
    return 'bar';
  },
  handleExampleActionCategory: function (actionName, actionData) {
    for (var index in actionData) {
      console.log(actionName + ' (' + Helpers.getId(actionName) + ')[' + index + ']: ' + actionData[index]);
    }
  }
}

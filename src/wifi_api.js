// Copyright (c) 2014 Jaguar Land Rover. All rights reserved.
//

// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
/* * /
function isInteger(value) {
  return isFinite(value) && !isNaN(parseInt(value));
}

function isString(value) {
  return typeof(value) === 'string' || value instanceof String;
}

function assertThrow(expr, exception) {
  if (!expr)
    throw new tizen.WebAPIException(tizen.WebAPIException[exception]);
}

function sendSyncMessage(cmd, msg) {
  msg['cmd'] = cmd;

  var serialized = JSON.stringify(msg);
  return JSON.parse(extension.internal.sendSyncMessage(serialized));
}

function NativeBridge() {
  this.listeners = {};
  this.next_listener_id = 0;
  this.queued_messages = {};
}

NativeBridge.prototype.requestLocalMessagePort = function(messagePortName) {
  return sendSyncMessage('RequestLocalMessagePort', {
    messagePortName: messagePortName,
    trusted: false
  });
};

NativeBridge.prototype.requestTrustedLocalMessagePort = function(messagePortName) {
  return sendSyncMessage('RequestLocalMessagePort', {
    messagePortName: messagePortName,
    trusted: true
  });
};

NativeBridge.prototype.requestRemoteMessagePort = function(appId, messagePortName) {
  return sendSyncMessage('RequestRemoteMessagePort', {
    appId: appId,
    messagePortName: messagePortName,
    trusted: false
  });
};

NativeBridge.prototype.requestTrustedRemoteMessagePort = function(appId, messagePortName) {
  return sendSyncMessage('RequestRemoteMessagePort', {
    appId: appId,
    messagePortName: messagePortName,
    trusted: true
  });
};

NativeBridge.prototype.addLocalListener = function(localPort, listenerFunc) {
  if (!this.listeners.hasOwnProperty(localPort._id))
    this.listeners[localPort._id] = [];

  this.next_listener_id++;
  this.listeners[localPort._id].push([listenerFunc, this.next_listener_id]);

  if (typeof(this.queued_messages[localPort.id]) !== 'undefined') {
    var queue = this.queued_messages[localPort.id];
    for (var i = 0, j = queue.length; i < j; i++)
      this.onLocalMessageReceived(queue[i]);
    this.queued_messages[localPort.id] = [];
  }

  return this.next_listener_id;
};

NativeBridge.prototype.removeLocalListener = function(localPort, watchId) {
  var listeners = this.listeners[localPort._id];
  if (typeof(listeners) === 'undefined')
    return { not_found: 1 };

  var to_delete = [];
  for (var i = 0, j = listeners.length; i < j; i++) {
    var listener_id = listeners[i][1];
    if (watchId == listener_id)
      to_delete.push(i);
  }

  if (to_delete.length == 0)
    return { not_found: 1 };

  for (var i = 0, j = to_delete.length; i < j; i++)
    this.listeners.splice(to_delete[i], 1);

  return {};
};

NativeBridge.prototype.onLocalMessageReceived = function(msg) {
  var listeners = this.listeners[msg.id];
  if (typeof(listeners) === 'undefined') {
    if (typeof(this.queued_messages[msg.id]) === 'undefined')
      this.queued_messages[msg.id] = [];
    this.queued_messages[msg.id].push(msg);
    return;
  }

  // FIXME(leandro): Reuse RemoteMessagePort if it has been requested
  // previously.
  var rmp = new RemoteMessagePort(msg.remotePort, msg.remoteAppId, msg.trusted);
  for (var i = 0; i < listeners.length; i++) {
    var func = listeners[i][0];
    func(msg.data, rmp);
  }
};

NativeBridge.prototype.sendMessage = function(remotePort, data, localPort) {
  return sendSyncMessage('SendMessage', {
    appId: remotePort.appId,
    messagePortName: remotePort.messagePortName,
    data: data,
    trusted: remotePort.isTrusted,
    localPort: localPort ? localPort._id : -1
  });
};

NativeBridge.prototype.toTizenException = function(nativeError) {
  function isUndefined(v) {
    return typeof(v) === 'undefined';
  }

  assertThrow(isUndefined(nativeError.not_found), 'NOT_FOUND_ERR');
  assertThrow(isUndefined(nativeError.invalid_parameter), 'INVALID_VALUES_ERR');
  assertThrow(isUndefined(nativeError.certificate_error), 'INVALID_ACCESS_ERR');
  assertThrow(isUndefined(nativeError.max_exceeded), 'QUOTA_EXCEEDED_ERR');
  assertThrow(nativeError.success === true, 'UNKNOWN_ERR');
};

var nativeBridge = new NativeBridge();

extension.setMessageListener(function(json) {
  var msg = JSON.parse(json);

  if (msg.cmd == 'LocalMessageReceived')
    nativeBridge.onLocalMessageReceived(msg);
  else
    console.error('Unknown command received: ' + msg.cmd);
});


function MessagePortManager() {
}

MessagePortManager.prototype.requestLocalMessagePort = function(
    localMessagePortName) {
  assertThrow(isString(localMessagePortName), 'TYPE_MISMATCH_ERR');

  var messagePort = nativeBridge.requestLocalMessagePort(
      localMessagePortName);
  nativeBridge.toTizenException(messagePort);

  return new LocalMessagePort(messagePort.id, localMessagePortName, false);
};

MessagePortManager.prototype.requestTrustedLocalMessagePort = function(
    localMessagePortName) {
  assertThrow(isString(localMessagePortName), 'TYPE_MISMATCH_ERR');

  var messagePort = nativeBridge.requestTrustedLocalMessagePort(
      localMessagePortName);
  nativeBridge.toTizenException(messagePort);

  return new LocalMessagePort(messagePort.id, localMessagePortName, true);
};

MessagePortManager.prototype.requestRemoteMessagePort = function(
    appId, remoteMessagePortName) {
  assertThrow(isString(appId), 'TYPE_MISMATCH_ERR');
  assertThrow(isString(remoteMessagePortName), 'TYPE_MISMATCH_ERR');

  var messagePort = nativeBridge.requestRemoteMessagePort(
      appId, remoteMessagePortName);
  nativeBridge.toTizenException(messagePort);

  return new RemoteMessagePort(remoteMessagePortName, appId, false);
};

MessagePortManager.prototype.requestTrustedRemoteMessagePort = function(
    appId, remoteMessagePortName) {
  assertThrow(isString(appId), 'TYPE_MISMATCH_ERR');
  assertThrow(isString(remoteMessagePortName), 'TYPE_MISMATCH_ERR');

  var messagePort = nativeBridge.requestTrustedRemoteMessagePort(appId,
      remoteMessagePortName);
  nativeBridge.toTizenException(messagePort);

  return new RemoteMessagePort(remoteMessagePortName, appId, true);
};

function LocalMessagePort(id, messagePortName, isTrusted) {
  Object.defineProperties(this, {
    '_id': { value: id, writable: false, enumerable: false },
    'messagePortName': { value: messagePortName, writable: false },
    'isTrusted': { value: !!isTrusted, writable: false }
  });
}

LocalMessagePort.prototype.addMessagePortListener = function(listener) {
  assertThrow(listener instanceof Function);

  return nativeBridge.addLocalListener(this, listener);
};

LocalMessagePort.prototype.removeMessagePortListener = function(watchId) {
  assertThrow(isInteger(watchId), 'TYPE_MISMATCH_ERR');
  assertThrow(watchId >= 0, 'INVALID_VALUES_ERR');

  var error = nativeBridge.removeLocalListener(this, watchId);
  nativeBridge.toTizenException(error);
};

function RemoteMessagePort(messagePortName, appId, isTrusted) {
  Object.defineProperties(this, {
    'messagePortName': { value: messagePortName, writable: false },
    'appId': { value: appId, writable: false },
    'isTrusted': { value: !!isTrusted, writable: false }
  });
}

RemoteMessagePort.prototype.sendMessage = function(data, localMessagePort) {
  assertThrow(data instanceof Array, 'TYPE_MISMATCH_ERR');
  if (arguments.length >= 2)
    assertThrow(localMessagePort instanceof LocalMessagePort, 'TYPE_MISMATCH_ERR');

  var filtered_data = new Array(data.length);
  try {
    for (var i = 0, j = data.length; i < j; i++)
      filtered_data[i] = { key: data[i].key, value: data[i].value };
  } catch (e) {
    assertThrow(Object.hasOwnProperty(data[i], 'key'), 'INVALID_VALUES_ERR');
    assertThrow(Object.hasOwnProperty(data[i], 'value'), 'INVALID_VALUES_ERR');
    throw new tizen.WebAPIException.UNKNOWN_ERR;
  }

  var error = nativeBridge.sendMessage(this, filtered_data, localMessagePort);
  nativeBridge.toTizenException(error);
};

var messagePortManagerObject = new MessagePortManager();
/* */


// usage: log('inside coolFunc',this,arguments);
// http://paulirish.com/2009/log-a-lightweight-wrapper-for-consolelog/
window.log = function(){
  log.history = log.history || [];   // store logs to an array for reference
  log.history.push(arguments);
  if(this.console){
    console.log( Array.prototype.slice.call(arguments) );
  }
};



// "Activate";
// "Deactivate";
// "Scan";
// "Connect";
// "Disconnect";
// "GetServices";



var WifiBridge = function() {

    return {
        HandleSetPowered: function(msg) {
            if (msg) {
                log('inside HandleSetPowered', msg);
            } else {
                log('inside HandleSetPowered: no msg.');
            }
        },
        HandleGetPowered: function(msg) {
            if (msg) {
                log('inside HandleGetPowered', msg);
            } else {
                log('inside HandleGetPowered: no msg.');
            }
        },
        SetTethering: function(msg) {
            if (msg) {
                log('inside SetTethering', msg);
            } else {
                log('inside SetTethering: no msg.');
            }
        },
        Connect: function(msg) {
            if (msg) {
                log('inside Connect', msg);
            } else {
                log('inside Connect: no msg.');
            }
        },
        Disconnect: function(msg) {
            if (msg) {
                log('inside Disconnect', msg);
            } else {
                log('inside Disconnect: no msg.');
            }
        },
        SetAutoConnect: function(msg) {
            if (msg) {
                log('inside SetAutoConnect', msg);
            } else {
                log('inside SetAutoConnect: no msg.');
            }
        },
        GetTechnologies: function(msg) {
            if (msg) {
                log('inside GetTechnologies', msg);
            } else {
                log('inside GetTechnologies: no msg.');
            }
        },
        GetServices: function(msg) {
            if (msg) {
                log('inside GetServices', msg);
            } else {
                log('inside GetServices: no msg.');
            }
        },
        Scan: function(msg) {
            if (msg) {
                log('inside Scan', msg);
            } else {
                log('inside Scan: no msg.');
            }
        }
    }
}


// my side... listening for responses from C++
extension.setMessageListener(function(json) {
    var msg = JSON.parse(json),
    cmd = (msg.cmd) ? msg.cmd: undefined;
    if (cmd) {
        switch (cmd) {
            case 'SetPowered':
                Wifi.HandleSetPowered(msg);
                break;
            case 'GetPowered':
                Wifi.HandleGetPowered(msg);
                break;
            case 'SetTethering':
                Wifi.SetTethering(msg);
                break;
            case 'Connect':
                Wifi.Connect(msg);
                break;
            case 'Disconnect':
                Wifi.Disconnect(msg);
                break;
            case 'SetAutoConnect':
                Wifi.SetAutoConnect(msg);
                break;
            case 'GetTechnologies':
                Wifi.GetTechnologies(msg);
                break;
            case 'GetServices':
                Wifi.GetServices(msg);
                break;
            case 'Scan':
                Wifi.Scan(msg);
                break;
        }
    }
});

/*
exports.setPowered = WifiBridge.SetPowered;
exports.getPowered = WifiBridge.GetPowered;
exports.setTethering = WifiBridge.SetTethering;
exports.connect = WifiBridge.Connect;
exports.disconnect = WifiBridge.Disconnect;
exports.setAutoConnect = WifiBridge.SetAutoConnect;
exports.getTechnologies = WifiBridge.GetTechnologies;
exports.getServices = WifiBridge.GetServices;
exports.scan = WifiBridge.Scan;
*/


#include "dbus_utils.h"
#include "dbus_helpers.h"
#include "gdbus/gdbus.h"

#include <dbus/dbus.h>
#include <glib.h>

#include <iostream>

using std::cout;
using std::endl;

// Turn Wifi On/Off
int
power_wifi( DBusConnection* connection, gboolean is_on )
{
    cout << "// power_wifi entry" << endl;

    dbus_bool_t option = is_on;

    return __connmanctl_dbus_set_property(
            connection,
            CONNMAN_WIFI_OBJECT_PATH,
            CONNMAN_TECHNOLOGY_INTERFACE,
            null_return,                // return function callback
            CONNMAN_WIFI_OBJECT_PATH,   // user_data, passed to send_method_call as argument for return callback
            "Powered",                  // property
            DBUS_TYPE_BOOLEAN,          // type
            &option );                  // value
}

// Scan Available Networks
int
do_scan( DBusConnection* connection )
{
    cout << "// do_scan entry" << endl;
    return __connmanctl_dbus_method_call(
            connection,
            CONNMAN_SERVICE,
            CONNMAN_WIFI_OBJECT_PATH,
            CONNMAN_TECHNOLOGY_INTERFACE,
            "Scan",                     // method
            null_return,                // return function callback
            CONNMAN_WIFI_OBJECT_PATH,   // user_data
            NULL,                       // no append function
            NULL);                      // no append data
}


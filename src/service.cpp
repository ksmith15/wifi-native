/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2012-2013  Intel Corporation. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

// #include "Connection.h"
//#include "Services.h"
#include "dbus_helpers.h"

// #include <gio/gio.h>

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>

#include <dbus/dbus.h>
#include <glib.h>

#include "gdbus/gdbus.h"

#include <iostream>
#include <iomanip>

using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::left;
using std::setw;

static gboolean opt_system_bus = FALSE;
const int TIMEOUT = 12000;

DBusPendingCall* get_services_pending = 0; // get_services_call; handle for dbus pending reply object

static int
list_services( DBusMessageIter* iter, const char* error, void* user_data )
{
    cout << "// list_services entry" << endl;
    if ( !error )
    {
        parse_service_array( iter );
    }
    else
    {
        cerr << "Error: " << error << endl;
    }

    cout << "// list_services exit" << endl;
    return 0;
}

void
get_services_notify( DBusPendingCall* pending, void* user_data ) // get_services_callback
{
    cout << "// get_services_notify entry" << endl;
    cout << "// check if GetServices call has completed" << endl;
    if ( dbus_pending_call_get_completed( pending ) == FALSE )
    {
        cout << "// get_services_notify exit with error in pending_call_get_completed" << endl;
        return;
    }

    get_services_pending = NULL;

    DBusMessageIter iter;
    DBusMessage* reply = dbus_pending_call_steal_reply( pending );
    if (( reply != NULL )
        && ( dbus_message_iter_init( reply, &iter ) != FALSE ))
    {
        cout << "// get_services_notify parsing reply" << endl;
        parse_service_array( &iter );
    }

    if ( reply != NULL )
    {
        cout << "// get_services_notify got empty reply" << endl;
        dbus_message_unref( reply );
    }

    cout << "// get_services_notify exit" << endl;
    dbus_pending_call_unref( pending );
}

int
get_services( DBusConnection* connection )
{
    cout << "// get_services entry with connection = " << connection << endl;
    int ret = 0;

    cout << "// get_services exit" << endl;
    return __connmanctl_dbus_method_call(
            connection,
            CONNMAN_SERVICE,
            CONNMAN_PATH,
            CONNMAN_MANAGER_INTERFACE,
            "GetServices",              // method
            list_services,              // method return callback
            NULL,
            NULL,
            NULL
            );
}


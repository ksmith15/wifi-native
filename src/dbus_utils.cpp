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

struct dbus_callback
{
    connmanctl_dbus_method_return_func_t cb;
    void* user_data;
};


const char*
print_dbus_type( int theType )
{
    char* text = 0;
    switch ( theType )
    {
    case DBUS_TYPE_INVALID:
        text = "dbus type INVALID";
        break;
    case DBUS_TYPE_INT16: case DBUS_TYPE_UINT16:
    case DBUS_TYPE_INT32: case DBUS_TYPE_UINT32:
    case DBUS_TYPE_INT64: case DBUS_TYPE_UINT64:
        text = "dbus type INTEGER";
        break;
    case DBUS_TYPE_STRING:
        text = "dbus type STRING";
        break;
    case DBUS_TYPE_VARIANT:
        text = "dbus type VARIANT";
        break;
    default:
        text = "dbus type report not implemented";
        break;
    }
    return text;
}

static void
on_name_appeared( DBusConnection* connection, const gchar* name, const gchar* name_owner, gpointer user_data )
{
    printf( "Name %s on %s is owned by %s\n",
            name, opt_system_bus ? "the system bus" : "the session bus",
            name_owner );
}

static void
on_name_vanished( DBusConnection* connection, const gchar* name, gpointer user_data )
{
    printf( "Name %s does not exist on %s\n", name, opt_system_bus ? "the system bus" : "the session bus" );
}

static int
null_return( DBusMessageIter* iter, const char* error, void* user_data )
{
//    g_free( user_data );
    return 0;
}

int
send_method_call( DBusConnection* connection, DBusMessage* message )
{
    cout << "// send_method_call entry" << endl;
    dbus_connection_send_with_reply( connection, message, &get_services_pending, TIMEOUT );
    dbus_message_unref( message );
    return 0;
}


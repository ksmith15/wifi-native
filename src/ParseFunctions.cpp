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
#include "Services.h"
#include "dbus_helpers.h"

#include <dbus/dbus.h>
#include <glib.h>

#include "gdbus/gdbus.h"

#include <iostream>

using std::cerr;
using std::cout;
using std::endl;

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

void
parse_property( DBusMessageIter* nvp_iter, nvp& property )
{
    cout << "// parse_property entry" << endl;
    char* name;
    char* value;
    DBusMessageIter val_iter;
    dbus_message_iter_get_basic( nvp_iter, &name );
    property.name_ = name;
    cout << "// parse_property: property name is '" << property.name_ << "'" << endl;
    dbus_message_iter_next( nvp_iter );
    cout << "// parse_property: value type "
         << print_dbus_type( dbus_message_iter_get_arg_type( nvp_iter ))
         << endl;
    if ( dbus_message_iter_get_arg_type( nvp_iter ) == DBUS_TYPE_VARIANT )
    {
        dbus_message_iter_recurse( nvp_iter, &val_iter );
        dbus_message_iter_get_basic( &val_iter, &value );
        property.value_ = value;
        cout << "// parse_property: value is " << property.value_ << endl;
    }
}

void
parse_property_dict( DBusMessageIter* dict_iter, nvplist& props )
{
    cout << "// parse_property_dict entry" << endl;
    DBusMessageIter nvp_iter;
    dbus_message_iter_recurse( dict_iter, &nvp_iter );
    while ( dbus_message_iter_get_arg_type( &nvp_iter ) != DBUS_TYPE_INVALID )
    {
        nvp pair;
        parse_property( &nvp_iter, pair );
        props.push_back( pair );
        dbus_message_iter_next( &nvp_iter );
    }
}

void
parse_service_struct( DBusMessageIter* array_iter, service& svc )
{
    cout << "// parse_service_struct entry" << endl;
    cout << "// parse_service_struct: first item in struct is an object_path" << endl;
    DBusMessageIter struct_iter;
    dbus_message_iter_recurse( array_iter, &struct_iter );
    if ( dbus_message_iter_get_arg_type( &struct_iter ) != DBUS_TYPE_OBJECT_PATH )
    {
        cout << "// parse_service_struct: no object_path found" << endl;
        return;
    }

    char* path = NULL;
    dbus_message_iter_get_basic( &struct_iter, &path );
    dbus_message_iter_next( &struct_iter );
    while ( dbus_message_iter_get_arg_type( &struct_iter ) == DBUS_TYPE_ARRAY )
    {
        cout << "// parse_service_struct: iterating into property array" << endl;
        DBusMessageIter dict_iter;
        dbus_message_iter_recurse( &struct_iter, &dict_iter );
        parse_property_dict( &dict_iter );
        dbus_message_iter_next( &struct_iter );
    }
}

/* a(oa{sv})
 * array of structs:
 *     object_path
 *     array of dicts:
 *         string : variant
 */
void
parse_service_array( DBusMessageIter* message_iter, servicelist& services )
{
    cout << "// parse_service_array entry" << endl;
    if ( dbus_message_iter_get_arg_type( message_iter ) != DBUS_TYPE_ARRAY )
    {
        return;
    }

    cout << "// recurse into struct" << endl;
    DBusMessageIter array_iter;
    for ( dbus_message_iter_recurse( message_iter, &array_iter );
          dbus_message_iter_get_arg_type( &array_iter ) == DBUS_TYPE_STRUCT;
          dbus_message_iter_next( &array_iter ))
    {
        service s;
        parse_service_struct( &array_iter, s );
        services.push_back( s );
    }
}

/*
int
list_services( DBusMessageIter* iter, const char* error, void* user_data )
{
    if ( !error )
    {
        parse_service_array( iter, (servicelist&)(*user_data) );
    }
    return 0;
}
*/


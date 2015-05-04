/* Copyright (C) 2015 Jaguar Land Rover - All Rights Reserved
 *
 * Proprietary and confidential
 * Unauthorized copying of this file, via any medium, is strictly prohibited
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * Filename:            WifiMaster.h
 * Version:             1.0
 * Project:             Tizen Crosswalk Wifi
 * Contributors:        Kenneth F. Smith <ksmith15@jaguarlandrover.com>
 *
 */

#include "WifiMaster.h"
#include "ParseFunctions.h"
#include "dbus_utils.h"
#include "dbus_helpers.h"

#include <list>
#include <string>

WifiMaster*
WifiMaster::Create()
{
    WifiMaster* wm = 0;
    wm = new WifiMaster;
    if ( wm )
        wm->initialize();
    return wm;
}

void
WifiMaster::initialize()
{
}

int
WifiMaster::Activate()
{
    const dbus_bool_t option = TRUE;
    return __connmanctl_dbus_set_property(
            connection_,
            CONNMAN_WIFI_OBJECT_PATH,
            CONNMAN_TECHNOLOGY_INTERFACE,
            null_return,
            CONNMAN_WIFI_OBJECT_PATH,
            "Powered",
            DBUS_TYPE_BOOLEAN,
            &option );
}

int
WifiMaster::Scan()
{
    return __connmanctl_dbus_method_call(
            connection_,
            CONNMAN_SERVICE,
            CONNMAN_WIFI_OBJECT_PATH,
            CONNMAN_TECHNOLOGY_INTERFACE,
            "Scan",
            null_return,
            CONNMAN_WIFI_OBJECT_PATH,
            NULL, NULL );
}

void
WifiMaster::Connect( const char* path )
{
    return __connmanctl_dbus_method_call(
            connection_,
            CONNMAN_SERVICE,
            path,
            "net.connman.Service",
            "Connect",
            null_return,
            path,
            NULL, NULL );
}

void
WifiMaster::Disconnect( const char* path )
{
    return __connmanctl_dbus_method_call(
            connection_,
            CONNMAN_SERVICE,
            path,
            "net.connman.Service",
            "Disonnect",
            null_return,
            path,
            NULL, NULL );
}

static int
list_services( DBusMessageIter* iter, const char* error, void* user_data )
{
    if ( !error )
    {
        parse_service_array( iter, *((servicelist*)user_data) );
    }
    return 0;
}

int
WifiMaster::GetNetworks()
{
    return __connmanctl_dbus_method_call(
            connection_,
            CONNMAN_SERVICE,
            CONNMAN_PATH,
            CONNMAN_MANAGER_INTERFACE,
            "GetServices",
            list_services,
            (void*)(&services_), NULL, NULL );
}

void
WifiMaster::GetNetworkInfo( std::string& netinfo )
{
    // create json string containing network info from list_services()
    netinfo = "{\n";
    servicelist::iterator i = services_.begin();
    servicelist::iterator i_fore = i;
    servicelist::iterator i_end = services_.end();
    for ( ; i != i_end; ++i )
    {
        netinfo += "\t\"";
        netinfo += i->path_;
        netinfo += "\" : {\n";
        nvplist::iterator j = i->props_.begin();
        nvplist::iterator j_fore = j;
        nvplist::iterator j_end = i->props_.end();
        for ( ; j != j_end; ++j )
        {
            netinfo += "\t\t\"";
            netinfo += j->name_;
            netinfo += "\" : \"";
            netinfo += j->value_;
            netinfo += "\"";
            ++j_fore;
            if ( j_fore == j_end ) netinfo += ",\n";
        }
        netinfo += "\n\t}";
        ++i_fore;
        if ( i_fore == i_end ) netinfo += ",\n";
    }
    netinfo += "\n}";
}

int
WifiMaster::Deactivate()
{
    const dbus_bool_t option = FALSE;
    return __connmanctl_dbus_set_property(
            connection_,
            CONNMAN_WIFI_OBJECT_PATH,
            CONNMAN_TECHNOLOGY_INTERFACE,
            null_return,
            CONNMAN_WIFI_OBJECT_PATH,
            "Powered",
            DBUS_TYPE_BOOLEAN,
            &option );
}


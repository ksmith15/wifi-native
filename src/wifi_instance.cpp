/* Copyright (C) Jaguar Land Rover - All Rights Reserved
 *
 * Proprietary and confidential
 * Unauthorized copying of this file, via any medium, is strictly prohibited
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 * Filename:            wifi_extension.cpp
 * Version:             1.0
 * Date:                December 2014
 * Project:             Tizen Crosswalk Instance Wifi
 * Contributors:        Kenneth F. Smith <ksmith15@jaguarlandrover.com>
 *
 */

#include "src/wifi_instance.h"
#include "src/wifi_extension.h"
#include "src/WifiMaster.h"
//#include "src/dbus_helpers.h"
//#include "src/dbus_utils.h"

#include <cstdio>
#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

namespace {
    const char* kCmdActivate = "Activate";
    const char* kCmdDeactivate = "Deactivate";
    const char* kCmdScan = "Scan";
    const char* kCmdConnect = "Connect";
    const char* kCmdDisconnect = "Disconnect";
    const char* kCmdGetServices = "GetServices";
}

static char gResponse[20] = {0};

static WifiMaster* WF_MASTER = 0;

WifiInstance::~WifiInstance()
{
    if ( tech_proxy_ )
    {
        g_object_unref( tech_proxy_ );
    }
    if ( service_proxy_ )
    {
        g_object_unref( service_proxy_ );
    }
    if ( manager_proxy_ )
    {
        g_object_unref( manager_proxy_ );
    }
    if ( agent_proxy_ )
    {
        g_object_unref( agent_proxy_ );
    }
}

void
WifiInstance::initialize()
{
    WF_MASTER = WifiMaster::Create();
    if ( ! WF_MASTER )
    {
        return;
    }

    g_dbus_proxy_new_for_bus( G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL,
            "net.connman",              // name
            "/net/connman/Manager",     // object path
            "net.connman.Manager",      // interface name
            NULL,                       // cancellable
            OnManagerProxyCreatedThunk, // callback
            this );

    g_dbus_proxy_new_for_bus( G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL,
            "net.connman",
            "/net/connman/Service",
            "net.connman.Service",
            NULL,
            OnServiceProxyCreatedThunk,
            this );

    g_dbus_proxy_new_for_bus( G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL,
            "net.connman",
            "/net/connman/Technology",
            "net.connman.Technology",
            NULL,
            OnTechnologyProxyCreatedThunk,
            this );

    g_dbus_proxy_new_for_bus( G_BUS_TYPE_SYSTEM,
            G_DBUS_PROXY_FLAGS_NONE,
            NULL,
            "net.connman",
            "/net/connman/Agent",
            "net.connman.Agent",
            NULL,
            OnAgentProxyCreatedThunk,
            this );

    g_dbus_object_manager_client_new_for_bus( G_BUS_TYPE_SYSTEM,
            G_DBUS_OBJECT_MANAGER_CLIENT_FLAGS_NONE,
            "net.connman",
            "/",
            NULL,
            NULL,
            NULL,
            NULL,
            OnManagerCreatedThunk,
            this );

//    WF_MASTER->Activate();
//    WF_MASTER->Scan();
//    return true;
}

void
WifiInstance::OnAgentProxyCreated( GObject*, GAsyncResult* result )
{
    GError* error = 0;
    agent_proxy_ = g_dbus_proxy_new_for_bus_finish( result, &error );
    if ( !agent_proxy_ )
    {
        g_printerr( "## agent_proxy_ creation error: %s\n", error->message );
        g_error_free( error );
        return;
    }

    gchar** properties = g_dbus_proxy_get_cached_property_names( agent_proxy_ );

    for ( unsigned int n = 0; properties && properties[n]; ++n )
    {
        const gchar* key = properties[n];
        GVariant* value = g_dbus_proxy_get_cached_property( agent_proxy_, key );
        gchar* value_str = g_variant_print( value, true );

        agent_info_[key] = value_str;
        g_variant_unref( value );
        g_free( value_str );
    }

    g_signal_connect( agent_proxy_, "g-properties-changed", G_CALLBACK( WifiInstance::OnPropertiesChanged ), this );
    g_strfreev( properties );
}

void
WifiInstance::OnManagerCreated( GObject*, GAsyncResult* result )
{
    GError* error = 0;
    object_manager_ = g_dbus_object_manager_client_new_for_bus_finish( result, &error );

    if ( !object_manager_ )
    {
        g_printerr( "## ObjectManager creation error: %s\n", error->message );
        g_error_free( error );
    }
    else
    {
        g_signal_connect( object_manager_, "object-added",
            G_CALLBACK(WifiInstance::OnDBusObjectAddedThunk), this );
        g_signal_connect( object_manager_, "object-removed",
            G_CALLBACK(WifiInstance::OnDBusObjectRemovedThunk), this );

        GList* managed_objects = g_dbus_object_manager_get_objects( object_manager_ );
        g_list_foreach( managed_objects, CacheManagedObject, this );
        g_list_free( managed_objects );
    }
}

void
WifiInstance::HandleMessage( const char* message )
{
    picojson::value v;
    string error;
    picojson::parse( v, message, message + strlen( message ), &error );
    if ( !error.empty() )
    {
        cout << "Ignoring message." << endl;
        return;
    }

    string command = v.get( "cmd" ).to_str();
    if ( command == kCmdActivate )
        HandleActivate( v );
    else if ( command == kCmdDeactivate )
        HandleDeactivate( v );
    else if ( command == kCmdScan )
        HandleDeactivate( v );
    else if ( command == kCmdConnect )
        HandleConnect( v );
    else if ( command == kCmdDisconnect )
        HandleDisconnect( v );
}

void
WifiInstance::HandleSyncMessage( const char* message )
{
    picojson::value v;
    string error;
    picojson::parse( v, message, message + strlen( message ), &error );
    if ( !error.empty() )
    {
        cout << "Ignoring sync message." << endl;
        return;
    }

    string command = v.get( "cmd" ).to_str();
    if ( command == kCmdGetServices )
        HandleGetServices( v );
}

void
WifiInstance::HandleActivate( const picojson::value& message )
{
    activate_callback_id_ = message.get( "reply_id" ).to_str();
    if ( tech_proxy_ )
    {
        g_dbus_proxy_call(
            tech_proxy_,
            "SetProperty",
            g_variant_new( "sb", "Powered", "TRUE" ),
            G_DBUS_CALL_FLAGS_NONE,
            20000,
            NULL, NULL, NULL );
    }
}

void
WifiInstance::HandleDeactivate( const picojson::value& message )
{
    deactivate_callback_id_ = message.get( "reply_id" ).to_str();
    if ( tech_proxy_ )
    {
        g_dbus_proxy_call(
            tech_proxy_,
            "SetProperty",
            g_variant_new( "sb", "Powered", "FALSE" ),
            G_DBUS_CALL_FLAGS_NONE,
            20000,
            NULL, NULL, NULL );
    }
}

void
WifiInstance::HandleScan( const picojson::value& message )
{
    scan_callback_id_ = message.get( "reply_id" ).to_str();
    if ( tech_proxy_ )
    {
        g_dbus_proxy_call(
            tech_proxy_,
            "Scan",
            NULL,                       // parameters
            G_DBUS_CALL_FLAGS_NONE,     // flags
            20000,                      // timeout
            NULL, NULL, NULL );
    }
}

int
WifiInstance::GetScanResults()
{
    return WF_MASTER->GetServices();
}

void
WifiInstance::HandleGetServices( const picojson::value& message )
{
    connect_callback_id_ = message.get( "reply_id" ).to_str();
    if ( manager_proxy_ )
    {
        g_dbus_proxy_call(
            manager_proxy_,
            "GetServices",
            NULL,                       // parameters
            G_DBUS_CALL_FLAGS_NONE,     // flags
            20000,                      // timeout
            NULL, NULL, NULL );
    }
}

void
WifiInstance::HandleConnect( const picojson::value& message )
{
    connect_callback_id_ = message.get( "reply_id" ).to_str();
    if ( service_proxy_ )
    {
        g_dbus_proxy_call(
            service_proxy_,
            "Connect",
            NULL,                       // parameters
            G_DBUS_CALL_FLAGS_NONE,     // flags
            20000,                      // timeout
            NULL, NULL, NULL );
    }
}

void
WifiInstance::HandleDisconnect( const picojson::value& message )
{
    disconnect_callback_id_ = message.get( "reply_id" ).to_str();
    if ( service_proxy_ )
    {
        g_dbus_proxy_call(
            service_proxy_,
            "Disconnect",
            NULL,                       // parameters
            G_DBUS_CALL_FLAGS_NONE,     // flags
            20000,                      // timeout
            NULL, NULL, NULL );
    }
}

void
WifiInstance::HandleGetNetworkInfo( const picojson::value& message )
{
//    WifiBssInfo* info = static_cast<WifiBssInfo*>(network_->GetAt( index ));
}


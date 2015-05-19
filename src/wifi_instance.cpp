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
//#include "src/WifiMaster.h"
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

// static WifiMaster* WF_MASTER = 0;

static void
getPropertyValue( const char* key, GVariant* value, picojson::value::object& o )
{
    char* value_str = g_variant_print( value, true );
    o[key] = picojson::value( value_str );
    g_free( value_str );
}

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
//    WF_MASTER = WifiMaster::Create();
//    if ( ! WF_MASTER )
//    {
//        return;
//    }

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
WifiInstance::OnPropertiesChanged( GDBusProxy* proxy, GVariant* changed_properties,
                                   const gchar* const* invalidated_properties,
                                   gpointer user_data )
{
    const char* interface = g_dbus_proxy_get_interface_name( proxy );
    WifiInstance* handler = reinterpret_cast<WifiInstance*>(user_data);

    if ( g_variant_n_children( changed_properties ) > 0 )
    {
        GVariantIter* iter;
        g_variant_get( changed_properties, "a{sv}", &iter );

        const gchar* key;
        GVariant* value;
        picojson::value::object o;
        while( g_variant_iter_loop( iter, "{&sv}", &key, &value ))
        {
            getPropertyValue( key, value, o );
        }

        picojson::value v( o );
        handler->InternalPostMessage( v );
    }
}

void
WifiInstance::OnManagerProxyCreated( GObject*, GAsyncResult* result )
{
    GError* error = 0;
    manager_proxy_ = g_dbus_proxy_new_for_bus_finish( result, &error );
    if ( !manager_proxy_ )
    {
        GetDefaultReply();
        g_printerr( "\n\n## manager_proxy_ creation error: %s\n", error->message );
        g_error_free( error );
        return;
    }
    g_signal_connect( manager_proxy_, "g-signal", G_CALLBACK( WifiInstance::OnSignal ), this );
}

void
WifiInstance::OnServiceProxyCreated( GObject*, GAsyncResult* result )
{
    GError* error = 0;
    service_proxy_ = g_dbus_proxy_new_for_bus_finish( result, &error );
    if ( !agent_proxy_ )
    {
        GetDefaultReply();
        g_printerr( "\n\n## service_proxy_ creation error: %s\n", error->message );
        g_error_free( error );
        return;
    }
    g_signal_connect( service_proxy_, "g-signal", G_CALLBACK( WifiInstance::OnSignal ), this );
}

void
WifiInstance::OnTechnologyProxyCreated( GObject*, GAsyncResult* result )
{
    GError* error = 0;
    tech_proxy_ = g_dbus_proxy_new_for_bus_finish( result, &error );
    if ( !tech_proxy_ )
    {
        GetDefaultReply();
        g_printerr( "\n\n## tech_proxy_ creation error: %s\n", error->message );
        g_error_free( error );
        return;
    }
    g_signal_connect( tech_proxy_, "g-signal", G_CALLBACK( WifiInstance::OnSignal ), this );
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
//    return WF_MASTER->GetServices();
    return 0;
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
            NULL,
            OnGetServicesThunk,
            NULL );
    }
}

void
WifiInstance::OnGetServices( GObject* object, GAsyncResult* result )
{
    GError* error = 0;
    GVariant* fin_result = g_dbus_proxy_call_finish( service_proxy_, result, &error );

    if ( !fin_result )
    {
        GetDefaultReply();
        g_printerr( "\n\nError GetServices: %s\n", error->message );
        g_error_free( error );
        return;
    }

    picojson::value::object o;
    o[ "cmd" ] = picojson::value("");

    const gchar* key;
    GVariantIter* it;
    GVariantIter* dit;
    char* value_str;
    g_variant_get( fin_result, "a(sa{sv})", &it );
    while ( g_variant_iter_loop( it, "sa{sv}", &key, &dit ))
    {
        picojson::value::array properties;
        const gchar* prop;
        GVariant* val;
        while ( g_variant_iter_loop( dit, "{sv}", &prop, &val ))
        {
            picojson::value::object entry;
            entry[prop] = picojson::value( g_variant_get_string( val, NULL ));
            properties.push_back( picojson::value( entry ));
        }
        o[key] = picojson::value( properties );
    }

    InternalPostMessage( picojson::value( o ));
    GetDefaultReply();
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

void
WifiInstance::OnSignal( GDBusProxy* proxy, gchar* sender, gchar* signal, GVariant* parameters, gpointer data )
{
}

void
WifiInstance::GetDefaultReply()
{
    picojson::value::object o;
    o["reply_id"] = picojson::value( default_reply_id_ );
    if ( !is_js_context_initialized_ )
    {
        is_js_context_initialized_ = true;
    }
    InternalSetSyncReply( picojson::value(o) );
    default_reply_id_.clear();
}

void
WifiInstance::FlushPendingMessages()
{
    if ( !msg_queue_.empty() )
    {
        MessageQueue::iterator it;
        for ( it = msg_queue_.begin(); it != msg_queue_.end(); ++it )
        {
            PostMessage( (*it).serialize().c_str() );
        }
    }
}

void
WifiInstance::InternalPostMessage( picojson::value v )
{
    if ( !is_js_context_initialized_ )
    {
        msg_queue_.push_back( v );
        return;
    }
}

void
WifiInstance::InternalSetSyncReply( picojson::value v )
{
    SendSyncReply( v.serialize().c_str() );
    FlushPendingMessages();
}


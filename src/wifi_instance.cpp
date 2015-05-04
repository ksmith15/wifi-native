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
#include "src/dbus_helpers.h"
#include "src/dbus_utils.h"

#include <cstdio>

static char gResponse[20] = {0};

static WifiMaster* WF_MASTER = 0;

WifiInstance::~WifiInstance()
{
    dbus_connection_unref( connection_ );
}

bool
WifiInstance::initialize()
{
    WF_MASTER = WifiMaster::Create();
    if ( ! WF_MASTER )
    {
        return false;
    }

    DBusError dbus_error;
    dbus_error_init( &dbus_error );
    connection_ = g_dbus_setup_bus( DBUS_BUS_SYSTEM, NULL, &dbus_error );
    if ( dbus_error_is_set( &dbus_error ))
    {
        return false;
    }

    connection_session_ = g_dbus_setup_bus( DBUS_BUS_SESSION, NULL, NULL );
    WF_MASTER->Activate();
    WF_MASTER->Scan();
    return true;
}

void
WifiInstance::ParseMessage( std::string& response, const char* message )
{
    // unpack message and delegate response to appropriate handler
    // if "scan" : HandleScan()
    // if "connect" : HandleConnect(i);
}

void
WifiInstance::HandleMessage( const char* message )
{
    std::string response;
    ParseMessage( response, message );
    PostMessage( response.c_str() );
}

void
WifiInstance::HandleSyncMessage( const char* message )
{
    std::string response;
    ParseMessage( response, message );
    SendSyncReply( response.c_str() );
}

void
WifiInstance::HandleActivate()
{
    int r = WF_MASTER->Activate();
    snprintf( gResponse, 20, "%d", r );
    SendSyncReply( gResponse );
}

void
WifiInstance::HandleDeactivate()
{
    int r = WF_MASTER->Deactivate();
    snprintf( gResponse, 20, "%d", r );
    SendSyncReply( gResponse );
}

void
WifiInstance::HandleScan()
{
    int r = WF_MASTER->Scan();
    snprintf( gResponse, 20, "%d", r );
    SendSyncReply( gResponse );
}

int
WifiInstance::GetScanResults()
{
    return WF_MASTER->GetNetworks();
}

void
WifiInstance::HandleGetNetworks()
{
    int r = GetScanResults();
    snprintf( gResponse, 20, "%d", r );
    SendSyncReply( gResponse );
}

void
WifiInstance::HandleConnect()
{
//    WifiBssInfo* info = static_cast<WifiBssInfo*>(network_->GetAt( index ));
    const char* path = "/";
    WF_MASTER->Connect( path );
}

void
WifiInstance::HandleGetNetworkInfo()
{
//    WifiBssInfo* info = static_cast<WifiBssInfo*>(network_->GetAt( index ));
}


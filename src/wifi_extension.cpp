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
 * Filename:            wifi_extension.cpp
 * Version:             1.0
 * Project:             Tizen Crosswalk Extension Wifi
 * Contributors:        Kenneth F. Smith <ksmith15@jaguarlandrover.com>
 *
*/

#include "src/wifi_extension.h"
#include "src/wifi_instance.h"
#include <syslog.h>

common::Extension*
CreateExtension()
{
    return new WifiExtension;
}

extern const char kSource_wifi_api[];

WifiExtension::WifiExtension()
{
    const char* entry_points[] = { NULL };
    SetExtraJSEntryPoints( entry_points );
    SetExtensionName( "tizen.wifi" );
    SetJavaScriptAPI( kSource_wifi_api );
    syslog( LOG_USER | LOG_DEBUG, "WifiExtension ctor" );
}

WifiExtension::~WifiExtension()
{}

common::Instance*
WifiExtension::CreateInstance()
{
    syslog( LOG_USER | LOG_DEBUG, "CreateInstance" );
    return new WifiInstance;
}


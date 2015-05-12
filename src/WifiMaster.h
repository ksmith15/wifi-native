#ifndef _WIFI_MASTER_H_
#define _WIFI_MASTER_H_

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

#include "ParseFunctions.h"
#include <string>

struct DBusConnection;

class WifiMaster
{
public:

    static WifiMaster* Create();

    ~WifiMaster() {}

    void initialize();

    int  Activate();
    int  Scan();
    void Connect( const char* path );
    void Disconnect( const char* path );
    int  GetServices();
    void GetNetworkInfo( std::string& netinfo );
    int  Deactivate();

protected:
    WifiMaster()
    : connection_(0), connection_session_(0)
    {}

private:

    WifiMaster( const WifiMaster& );
    WifiMaster& operator=( WifiMaster& );

    DBusConnection* connection_;
    DBusConnection* connection_session_;
    servicelist     services_;
};

#endif // _WIFI_H_


#ifndef _WIFI_INSTANCE_H_
#define _WIFI_INSTANCE_H_

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
 * Filename:            wifi_extension.h
 * Version:             1.0
 * Project:             Tizen Crosswalk Instance Wifi
 * Contributors:        Kenneth F. Smith <ksmith15@jaguarlandrover.com>
 *
 */

#include "common/extension.h"

#define CONNMAN_SERVICE "net.connman"
#define CONNMAN_AGENT_INTERFACE CONNMAN_SERVICE ".Agent"
#define CONNMAN_MANAGER_INTERFACE CONNMAN_SERVICE ".Manager"
#define CONNMAN_TECHNOLOGY_INTERFACE CONNMAN_SERVICE ".Technology"

#define CONNMAN_MANAGER_PATH "/"
#define CONNMAN_SERVICE_PATH "/wifi"

struct DBusConnection;

class WifiInstance : public common::Instance
{
public:

    WifiInstance() {} //: wifi_mgr_(0), network_(0) {}
    virtual ~WifiInstance();
    bool initialize();

private:

    DBusConnection* connection_;
    DBusConnection* connection_session_;

    void ParseMessage( std::string& response, const char* message );

    virtual void HandleMessage( const char* msg );
    virtual void HandleSyncMessage( const char* msg );

    void HandleActivate();
    void HandleDeactivate();
    void HandleScan();
    int  GetScanResults();
    void HandleGetNetworks();
    void HandleGetNetworkInfo();
    void HandleConnect();

//    Tizen::Net::Wifi::WifiManager*  wifi_mgr_;
//    Tizen::Base::Collection::IList* network_;
};

#endif // _WIFI_INSTANCE_H_


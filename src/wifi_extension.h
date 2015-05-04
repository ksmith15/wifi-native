#ifndef _WIFI_EXTENSION_H_
#define _WIFI_EXTENSION_H_

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
 * Project:             Tizen Crosswalk Extension Wifi
 * Contributors:        Kenneth F. Smith <ksmith15@jaguarlandrover.com>
 *
*/

#include "common/extension.h"

class WifiExtension : public common::Extension
{
public:
    WifiExtension();
    virtual ~WifiExtension();

private:
    virtual common::Instance* CreateInstance();
};

#endif // _WIFI_EXTENSION_H_


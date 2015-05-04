#ifndef _WIFI_EXT_PARSE_FUNCTIONS_H_
#define _WIFI_EXT_PARSE_FUNCTIONS_H_

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

#include <list>
#include <string>

struct nvp
{
    nvp() {}
    nvp( const nvp& a ) : name_(a.name_), value_(a.value_) {}
    ~nvp() {}

    nvp& operator=( const nvp& a )
    {
        if ( this != &a )
        {
            this->~nvp();
            new (this) nvp(a);
        }
        return *this;
    }

    std::string name_;
    std::string value_;
};

typedef std::list<nvp> nvplist;

struct service
{
    std::string path_;
    nvplist     props_;
};

typedef std::list<service> servicelist;

struct DBusMessageIter;

const char* print_dbus_type( int theType );

void parse_property( DBusMessageIter* nvp_iter, nvp& property );

void parse_property_dict( DBusMessageIter* dict_iter, nvplist& props );

void parse_service_struct( DBusMessageIter* array_iter, service& svc );

void parse_service_array( DBusMessageIter* message_iter, servicelist& services );

#endif


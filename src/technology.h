#ifndef _TECHNOLOGY_H_
#define _TECHNOLOGY_H_

#include <glib.h>

struct DBusConnection;

int power_wifi( DBusConnection* connection, gboolean is_on );

int do_scan( DBusConnection* connection );

#endif


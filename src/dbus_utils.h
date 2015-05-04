#ifndef _WIFI_TECHNOLOGY_H_
#define _WIFI_TECHNOLOGY_H_

#include "dbus_helpers.h"

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <stdexcept>

#include <dbus/dbus.h>
#include <glib.h>

#include "gdbus/gdbus.h"

#define CONNMAN_SERVICE "net.connman"

#define CONNMAN_AGENT_INTERFACE         CONNMAN_SERVICE ".Agent"
#define CONNMAN_MANAGER_INTERFACE       CONNMAN_SERVICE ".Manager"
#define CONNMAN_SERVICE_INTERFACE       CONNMAN_SERVICE ".Service"
#define CONNMAN_TECHNOLOGY_INTERFACE    CONNMAN_SERVICE ".Technology"

#define CONNMAN_MANAGER_PATH        "/"
#define CONNMAN_SERVICE_PATH        "/wifi"
#define CONNMAN_WIFI_OBJECT_PATH    "/net/connman/technology/wifi"

struct dbus_callback
{
    connmanctl_dbus_method_return_func_t cb;
    void* user_data;
};

const char* print_dbus_type( int theType );

void on_name_appeared( DBusConnection* connection, const gchar* name, const gchar* name_owner, gpointer user_data );

void on_name_vanished( DBusConnection* connection, const gchar* name, gpointer user_data );

int null_return( DBusMessageIter* iter, const char* error, void* user_data );

int send_method_call( DBusConnection* connection, DBusMessage* message );

#endif


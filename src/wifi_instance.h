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
#include "common/picojson.h"

#include <gio/gio.h>
#include <string>
#include <map>

#define G_CALLBACK_1(METHOD, SENDER, ARG0)                                     \
    static void METHOD ## Thunk(SENDER sender, ARG0 res, gpointer userdata) {  \
        return reinterpret_cast<WifiInstance*>(userdata)->METHOD(sender,       \
                                                                 res);         \
    }                                                                          \
                                                                               \
    void METHOD(SENDER, ARG0)

#define G_CALLBACK_CANCELLABLE_1(METHOD, SENDER, ARG0)                         \
    static void METHOD ## Thunk(SENDER sender, ARG0 res, gpointer userdata) {  \
        ContextCancellable* context =                                          \
            reinterpret_cast<ContextCancellable*>(userdata);                   \
        if (context->cancellable                                               \
            && g_cancellable_is_cancelled(context->cancellable))               \
            goto done;                                                         \
        reinterpret_cast<WifiInstance*>(context->userdata)->METHOD(sender,     \
                                                                   res);       \
    done:                                                                      \
        delete context;                                                        \
    }                                                                          \
                                                                               \
  void METHOD(SENDER, ARG0);

struct ContextCancellable
{
    GCancellable* cancellable;
    void*         userdata;
};

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

    WifiInstance()
    : tech_proxy_(0), service_proxy_(0), manager_proxy_(0), agent_proxy_(0)
    {}

    virtual ~WifiInstance();
    void initialize();

    G_CALLBACK_1( OnAgentProxyCreated, GObject*, GAsyncResult*);
    G_CALLBACK_1( OnManagerProxyCreated, GObject*, GAsyncResult*);
    G_CALLBACK_1( OnServiceProxyCreated, GObject*, GAsyncResult*);
    G_CALLBACK_1( OnTechnologyProxyCreated, GObject*, GAsyncResult*);
    G_CALLBACK_1( OnManagerCreated, GObject*, GAsyncResult*);

    G_CALLBACK_CANCELLABLE_1( OnGetServices, GObject*, GAsyncResult*);

//    G_CALLBACK_1( OnDBusObjectAdded, GDBusObjectManager*, GDBusObject*);
//    G_CALLBACK_1( OnDBusObjectRemoved, GDBusObjectManager*, GDBusObject*);

private:

//    DBusConnection* connection_;
//    DBusConnection* connection_session_;

//    void ParseMessage( std::string& response, const char* message );

    virtual void HandleMessage( const char* msg );
    virtual void HandleSyncMessage( const char* msg );

    void HandleActivate( const picojson::value& msg );
    void HandleDeactivate( const picojson::value& msg );
    void HandleScan( const picojson::value& msg );
    int  GetScanResults();
    void HandleGetServices( const picojson::value& msg );
    void HandleGetNetworkInfo( const picojson::value& msg );
    void HandleConnect( const picojson::value& msg );
    void HandleDisconnect( const picojson::value& msg );

    static void OnPropertiesChanged( GDBusProxy*, GVariant* changed_properties, const gchar* const* invalidated_properties, gpointer user_data );
    // static void CacheManagedObject( gpointer data, gpointer user_data );
    static void OnSignal( GDBusProxy* proxy, gchar* sender, gchar* signal, GVariant* parameters, gpointer data );

//    void OnGetServices( GObject* object, GAsyncResult* result );

    void GetDefaultReply();
    void FlushPendingMessages();
    void InternalPostMessage( picojson::value v );
    void InternalSetSyncReply( picojson::value v );

    typedef std::vector<picojson::value> MessageQueue;
    MessageQueue msg_queue_;
    std::map<std::string, std::string> callbacks_map_;

    bool is_js_context_initialized_;
    std::string default_reply_id_;

    GDBusProxy* tech_proxy_;
    GDBusProxy* service_proxy_;
    GDBusProxy* manager_proxy_;
    GDBusProxy* agent_proxy_;

    std::string activate_callback_id_;
    std::string deactivate_callback_id_;
    std::string scan_callback_id_;
    std::string connect_callback_id_;
    std::string disconnect_callback_id_;

    std::map<std::string, std::string> agent_info_;

//    Tizen::Net::Wifi::WifiManager*  wifi_mgr_;
//    Tizen::Base::Collection::IList* network_;
};

#endif // _WIFI_INSTANCE_H_


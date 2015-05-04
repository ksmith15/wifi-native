/*
 *
 *  Connection Manager
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <errno.h>
#include <glib.h>

// Remove input.h dependencies because this is not connmanctl...
// #include "input.h"
#include "dbus_helpers.h"

#define TIMEOUT         120000

struct dbus_callback {
	connmanctl_dbus_method_return_func_t cb;
	void *user_data;
};

static void dbus_method_reply(DBusPendingCall *call, void *user_data)
{
	struct dbus_callback *callback = user_data;
	int res = 0;
	DBusMessage *reply;
	DBusMessageIter iter;

	reply = dbus_pending_call_steal_reply(call);
	if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR) {
		DBusError err;

		dbus_error_init(&err);
		dbus_set_error_from_message(&err, reply);

		callback->cb(NULL, err.message, callback->user_data);

		dbus_error_free(&err);
	}
    else
    {
        dbus_message_iter_init(reply, &iter);
        res = callback->cb(&iter, NULL, callback->user_data);
    }

	g_free(callback);
	dbus_message_unref(reply);
}

static int send_method_call(DBusConnection *connection,
		DBusMessage *message, connmanctl_dbus_method_return_func_t cb,
		void *user_data)
{
	int res = -ENXIO;
	DBusPendingCall *call;
	struct dbus_callback *callback;

	if (!dbus_connection_send_with_reply(connection, message, &call, TIMEOUT))
		goto end;

	if (!call)
		goto end;

	if (cb) {
		callback = g_new0(struct dbus_callback, 1);
		callback->cb = cb;
		callback->user_data = user_data;
		dbus_pending_call_set_notify(call, dbus_method_reply,
				callback, NULL);
		res = -EINPROGRESS;
	}

end:
        dbus_message_unref(message);
	return res;
}

static int append_variant(DBusMessageIter *iter, const char *property,
		int type, void *value)
{
	DBusMessageIter variant;
	char *type_str;

	switch(type) {
	case DBUS_TYPE_BOOLEAN:
                type_str = DBUS_TYPE_BOOLEAN_AS_STRING;
                break;
        case DBUS_TYPE_BYTE:
                type_str = DBUS_TYPE_BYTE_AS_STRING;
                break;
        case DBUS_TYPE_STRING:
                type_str = DBUS_TYPE_STRING_AS_STRING;
                break;
	case DBUS_TYPE_INT32:
		type_str = DBUS_TYPE_INT32_AS_STRING;
		break;
	default:
		return -EOPNOTSUPP;
	}

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, type_str,
			&variant);
	dbus_message_iter_append_basic(&variant, type, value);
	dbus_message_iter_close_container(iter, &variant);

	return 0;
}

int __connmanctl_dbus_method_call(DBusConnection *connection,
		const char *service, const char *path, const char *interface,
		const char *method, connmanctl_dbus_method_return_func_t cb,
		void *user_data, connmanctl_dbus_append_func_t append_func,
		void *append_data)
{
	DBusMessage *message;
	DBusMessageIter iter;

	message = dbus_message_new_method_call(service, path, interface,
			method);

	if (!message)
		return -ENOMEM;

	if (append_func) {
		dbus_message_iter_init_append(message, &iter);
		append_func(&iter, append_data);
	}

	return send_method_call(connection, message, cb, user_data);
}

int __connmanctl_dbus_set_property(DBusConnection *connection,
		const char *path, const char *interface,
		connmanctl_dbus_method_return_func_t cb, void * user_data,
		const char *property, int type, void *value)
{
	DBusMessage *message;
	DBusMessageIter iter;

	message = dbus_message_new_method_call("net.connman", path,
			interface, "SetProperty");

	if (!message)
		return -ENOMEM;

	dbus_message_iter_init_append(message, &iter);

	if (append_variant(&iter, property, type, value) < 0) {
		dbus_message_unref(message);
		return -EINVAL;
	}

	return send_method_call(connection, message, cb, user_data);
}

void __connmanctl_dbus_append_dict(DBusMessageIter *iter,
		connmanctl_dbus_append_func_t append_fn, void *append_data)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
                        DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
                        DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
                        DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	if (append_fn)
		append_fn(&dict, append_data);

	dbus_message_iter_close_container(iter, &dict);
}

void __connmanctl_dbus_append_dict_entry(DBusMessageIter *iter,
		const char *property, int type, void *value)
{
	DBusMessageIter dict_entry;

	dbus_message_iter_open_container(iter, DBUS_TYPE_DICT_ENTRY, NULL,
			&dict_entry);

	append_variant(&dict_entry, property, type, value);

	dbus_message_iter_close_container(iter, &dict_entry);
}

int __connmanctl_dbus_set_property_dict(DBusConnection *connection,
		const char *path, const char *interface,
		connmanctl_dbus_method_return_func_t cb, void *user_data,
		const char *property, int type,
		connmanctl_dbus_append_func_t append_fn,
		void *append_user_data)
{
	DBusMessage *message;
	DBusMessageIter iter, variant, dict;

	message = dbus_message_new_method_call("net.connman", path,
			interface, "SetProperty");

	if (!message)
		return -ENOMEM;

	dbus_message_iter_init_append(message, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT,
			DBUS_TYPE_ARRAY_AS_STRING
				DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
				DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
			&variant);

	dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_STRING_AS_STRING
				DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
			&dict);

	append_fn(&dict, append_user_data);

	dbus_message_iter_close_container(&variant, &dict);
	dbus_message_iter_close_container(&iter, &variant);

	return send_method_call(connection, message, cb, user_data);
}

static void append_variant_array(DBusMessageIter *iter, const char *property,
		connmanctl_dbus_append_func_t append_fn,
		void *append_user_data)
{
	DBusMessageIter variant, array;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
			DBUS_TYPE_ARRAY_AS_STRING
				DBUS_TYPE_STRING_AS_STRING,
			&variant);

	dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY,
			DBUS_TYPE_STRING_AS_STRING, &array);

	append_fn(&array, append_user_data);

	dbus_message_iter_close_container(&variant, &array);
	dbus_message_iter_close_container(iter, &variant);
}

void __connmanctl_dbus_append_dict_string_array(DBusMessageIter *iter,
		const char *property, connmanctl_dbus_append_func_t append_fn,
		void *append_user_data)
{
	DBusMessageIter dict_entry;

	dbus_message_iter_open_container(iter, DBUS_TYPE_DICT_ENTRY, NULL,
			&dict_entry);

	append_variant_array(&dict_entry, property, append_fn,
			append_user_data);

	dbus_message_iter_close_container(iter, &dict_entry);
}

int __connmanctl_dbus_set_property_array(DBusConnection *connection,
		const char *path, const char *interface,
		connmanctl_dbus_method_return_func_t cb, void *user_data,
		const char *property, int type,
		connmanctl_dbus_append_func_t append_fn,
		void *append_user_data)
{
	DBusMessage *message;
	DBusMessageIter iter;

	if (type != DBUS_TYPE_STRING)
		return -EOPNOTSUPP;

	message = dbus_message_new_method_call("net.connman", path,
			interface, "SetProperty");

	if (!message)
		return -ENOMEM;

	dbus_message_iter_init_append(message, &iter);

	append_variant_array(&iter, property, append_fn, append_user_data);

	return send_method_call(connection, message, cb, user_data);
}

int __connmanctl_dbus_session_change(DBusConnection *connection,
		const char *session_path,
		connmanctl_dbus_method_return_func_t cb, void * user_data,
		const char *property, int type, void *value)
{
	DBusMessage *message;
	DBusMessageIter iter;

	message = dbus_message_new_method_call("net.connman", session_path,
			"net.connman.Session", "Change");

	if (!message)
		return -ENOMEM;

	dbus_message_iter_init_append(message, &iter);

	if (append_variant(&iter, property, type, value) < 0) {
		dbus_message_unref(message);
		return -EINVAL;
	}

	return send_method_call(connection, message, cb, user_data);
}

int __connmanctl_dbus_session_change_array(DBusConnection *connection,
		const char *session_path,
		connmanctl_dbus_method_return_func_t cb, void *user_data,
		const char *property,
		connmanctl_dbus_append_func_t append_fn,
		void *append_user_data)
{
	DBusMessage *message;
	DBusMessageIter iter;

	message = dbus_message_new_method_call("net.connman", session_path,
			"net.connman.Session", "Change");

	if (!message)
		return -ENOMEM;

	dbus_message_iter_init_append(message, &iter);

	append_variant_array(&iter, property, append_fn, append_user_data);

	return send_method_call(connection, message, cb, user_data);
}


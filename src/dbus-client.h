/*
 * httpdbusd - a http dbus bridge daemon
 *
 * Copyright © 2011 Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU Lesser General Public License,
 * version 2.1, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <://www.gnu.org/licenses>
 */

#ifndef __DBUS_CLIENT_H__
#define __DBUS_CLIENT_H__

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef struct _DBusClient DBusClient;

struct _DBusClient
{
  GDBusConnection *connection;
  GDBusProxy *proxy;
  GHashTable *interface;
};

DBusClient *dbus_client_new (const gchar *service_name,
                             const gchar *object_path,
                             const gchar *interface);

void
dbus_client_call (DBusClient *dbus_client,
                  const gchar *method,
                  const gchar *data);


void dbus_client_free (DBusClient *dbus_client);

G_END_DECLS

#endif

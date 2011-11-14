/*
 * dbus-http-bridge - a bridge for dbus to webservice
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
 * along with this program; if not, see <http://www.gnu.org/licenses>
 */

#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include "dbus-client.h"

static GDBusProxy *
dbus_client_proxy_new (DBusClient *dbus_client)
{
  GError *error=NULL;
  GDBusProxy *proxy;

  if (dbus_client->mex_player)
    g_object_unref (dbus_client->mex_player);

  proxy = g_dbus_proxy_new_sync (dbus_client->connection,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 NULL,
                                 MEX_PLAYER_SERVICE_NAME,
                                 MEX_PLAYER_OBJECT_PATH,
                                 MEX_PLAYER_INTERFACE_NAME,
                                 NULL,
                                 &error);
  if (error)
    {
      g_warning ("Could not create media player proxy: %s", error->message);
      g_error_free (error);
    }
  return proxy;
}

DBusClient *dbus_client_new (const gchar *service_name,
                             const gchar *object_path,
                             const gchar *interface)
{
  DBusClient *dbus_client;
  GError *error = NULL;

  dbus_client = g_new0 (DBusClient, 1);

  dbus_client->mex_input = NULL;
  dbus_client->mex_player = NULL;

  dbus_client->connection = g_bus_get_sync (G_BUS_TYPE_SESSION,
                                               NULL, &error);
  if (error)
    {
      g_warning ("Failed to connect to dbus %s\n",
               error->message ? error->message : "Unknown");
      g_error_free (error);
    }
  else
    {
      dbus_client->proxy = dbus_input_proxy_new (dbus_client);
     }
  return dbus_client;
}


void dbus_client_free (DBusClient *dbus_client)
{
  g_object_unref (dbus_client->connection);
  g_object_unref (dbus_client->proxy);
  g_free (dbus_client);
}

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
 *
 * Written by: Michael Wood <michael.g.wood@linux.intel.com>
 */

#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include "dbus-client.h"

GVariant *
parse_http_data (DBusClient *dbus_client, const gchar *data)
{
  GString *format_string;

  gchar *items;
  gint i;

  items = g_strsplit (data, "&", 0);


/* item[n] playing=false */ /* maybe lets have the args in a hashtable so that we can look up "playing" and see that it requires a "b" boolean */
  /*
  g_string_append (format_string, "(");
  for (i=0; (arg_info_itr = arg_info->in_args[i]); i++)
    {
      g_string_append (format_string, arg_info->signature);
    }
  g_string_append (format_string, ")");


  for (i=0; (items = items[i]); i++)
    {
      g_variant_new (
                     {
                     }*/

  GVariantBuilder *builder;

  builder = g_variant_builder_new (G_VARIANT_TYPE_ARRAY);
  for (i = 0; i < 16; i++)
    {
      gchar buf[3];

      sprintf (buf, "%d", i);
      g_variant_builder_add (builder, "{is}", i, buf);
    }

  return g_variant_builder_end (builder);

}

void
dbus_client_call (DBusClient *dbus_client,
                  const gchar *method,
                  const gchar *data)
{
  GDBusArgInfo *arg_info;
  gboolean found_key;
  GError *error = NULL;

  found_key = g_hash_table_lookup_extended (dbus_client->interface,
                                            method,
                                            NULL,
                                            &arg_info);

  /* The method was never inserted into the hash table */
  if (!found_key)
    return;
/* No arguments required */
  if (!arg_info)
    g_dbus_proxy_call_sync (dbus_client->proxy, method, NULL, 0, -1, NULL,
                            &error);
  else
    {
      GVariant *arg_data;
      arg_data = parse_http_data (arg_info, data);
      g_dbus_proxy_call_sync (dbus_client->proxy,
                              method,
                              data,
                              0, -1, NULL, &error);
    }


  if (error)
    {
      g_warning ("problem calling %s: %s", method, error->message);
      g_error_free (error);
    }
}

static GDBusProxy *
dbus_client_proxy_new (DBusClient *dbus_client,
                       const gchar *service_name,
                       const gchar *object_path,
                       const gchar *interface)
{
  GError *error=NULL;
  GDBusProxy *proxy;

  if (dbus_client->proxy)
    g_object_unref (dbus_client->proxy);

  g_debug ("asking for %s, %s, %s", service_name, object_path, interface);

  proxy = g_dbus_proxy_new_sync (dbus_client->connection,
                                 G_DBUS_PROXY_FLAGS_NONE,
                                 NULL,
                                 service_name,
                                 object_path,
                                 interface,
                                 NULL,
                                 &error);
  if (error)
    {
      g_warning ("Could not create dbus proxy: %s", error->message);
      g_error_free (error);
    }
  return proxy;
}

static void
dbus_client_inspect (DBusClient *dbus_client)
{
  GDBusInterfaceInfo *info;
  GDBusMethodInfo *method_info;
  GDBusArgInfo *arg_info;
  GError *error=NULL;
  gint i=0,j;
  GVariant *result;
  const gchar *xml_introspec_data;
  GDBusNodeInfo *node_info;

  result =
    g_dbus_proxy_call_sync (dbus_client->proxy,
                            "org.freedesktop.DBus.Introspectable.Introspect",
                            NULL,
                            G_DBUS_CALL_FLAGS_NONE,
                            -1,
                            NULL,
                            &error);

  if (error)
    {
      g_critical ("Cannot find introspection data %s", error->message);
      g_error_free (error);
      return;
    }

  g_variant_get (result, "(&s)", &xml_introspec_data);

  node_info = g_dbus_node_info_new_for_xml (xml_introspec_data, &error);

  if (error)
    {
      g_critical ("Cannot parse introspection data %s", error->message);
      g_error_free (error);
    }

  /* for now we only care about one interface's info */
  info = g_dbus_node_info_lookup_interface (node_info,
                         g_dbus_proxy_get_interface_name (dbus_client->proxy));

  dbus_client->interface = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  NULL); /* TODO value freer */

  g_debug ("interface: %s", info->name);

  GHashTable *arg_info_table;

  for (i=0; (method_info = info->methods[i]); i++)
    {

      g_debug (" - %s", method_info->name);
      if (method_info->in_args)
        {
          arg_info_table = g_hash_table_new (g_str_hash, g_str_equal);

          for (j=0; (arg_info = method_info->in_args[j]); j++)
            {
              g_debug ("    -- %s %s", arg_info->signature, arg_info->name);
              g_hash_table_insert (arg_info_table, arg_info->name,
                                   arg_info->signature);
            }
        }

      g_hash_table_insert (dbus_client->interface,
                           method_info->name,
                           arg_info_table);
    }
}


DBusClient *
dbus_client_new (const gchar *service_name,
                 const gchar *object_path,
                 const gchar *interface)
{
  DBusClient *dbus_client;
  GError *error = NULL;

  dbus_client = g_new0 (DBusClient, 1);

  dbus_client->proxy = NULL;

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
      dbus_client->proxy = dbus_client_proxy_new (dbus_client,
                                                  service_name,
                                                  object_path,
                                                  interface);
      if (dbus_client->proxy)
        {
          dbus_client_inspect (dbus_client);
        }

    }
  return dbus_client;
}


void dbus_client_free (DBusClient *dbus_client)
{
  g_object_unref (dbus_client->connection);
  g_object_unref (dbus_client->proxy);
  g_free (dbus_client);
}

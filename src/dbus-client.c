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

static GVariant *
parse_http_data (gpointer args_hash_table, const gchar *http_data)
{
  /* We could have used the query hashtable in the soup server cb but we
   * don't have that for POST requests, plus this makes memory mgmt easier
   */
  GHashTable *parsed_http_data, *arg_info_table;
  gchar **key_value_pairs;
  const gchar *itr;
  gint i;
  GHashTableIter iter;
  gpointer arg_name, arg_type;

  GVariantBuilder *gvbuilder;

  arg_info_table = (GHashTable *)args_hash_table;

  key_value_pairs = g_strsplit (http_data, "&", 0);

  parsed_http_data = g_hash_table_new_full (g_str_hash,
                                            g_str_equal,
                                            g_free,
                                            g_free);

  /* Put the http data into a hash table
   * This means that it can come to us in any order.
   */
  for (i=0; (itr = key_value_pairs[i]); i++)
    {
      gchar **value_key_pair;

      value_key_pair = g_strsplit (itr, "=", 2);

      g_hash_table_insert (parsed_http_data,
                           value_key_pair[0],
                           value_key_pair[1]);
    }

  gvbuilder = g_variant_builder_new (G_VARIANT_TYPE_TUPLE);
/* look at g_variant_parse... */
/* FIXME: The itr over the hashtable seems to go backwards up the table
  * would it be crazy to add the args in the reverse order.. */


  g_hash_table_iter_init (&iter, arg_info_table);
  while (g_hash_table_iter_next (&iter, &arg_name, &arg_type)) 
      {
        const gchar *http_value;
        /* lookup the key "this" in /methd?this=that in the http parsed data */
        http_value = g_hash_table_lookup (parsed_http_data,
                                          (const gchar *)arg_name);

        g_debug ("arg_name: %s, type: %s http value for this arg_name: %s",
                (const gchar *) arg_name, (const gchar *) arg_type,
                http_value);

        /* quick dirty type hacks */
        if (g_strcmp0 ((const gchar *)arg_type, "b") == 0)
          {
            if (g_strcmp0 (http_value, "0") == 0 ||
                g_strcmp0 (http_value, "false") == 0 ||
                g_strcmp0 (http_value, "FALSE") == 0)
              {
                g_variant_builder_add (gvbuilder, (const gchar*)arg_type,
                                       FALSE);
              }
            else
              {
                g_variant_builder_add (gvbuilder, (const gchar*)arg_type,
                                       TRUE);
              }
          }
        /* oh dear all int types fudged */
        else if (g_strcmp0 ((const gchar *)arg_type, "n") == 0 ||
                 g_strcmp0 ((const gchar *)arg_type, "q") == 0 ||
                 g_strcmp0 ((const gchar *)arg_type, "i") == 0 ||
                 g_strcmp0 ((const gchar *)arg_type, "u") == 0 ||
                 g_strcmp0 ((const gchar *)arg_type, "x") == 0 ||
                 g_strcmp0 ((const gchar *)arg_type, "t") == 0 ||
                 g_strcmp0 ((const gchar *)arg_type, "h") == 0)
          {
            int a;
            a = strtol (http_value, NULL, 0);
            g_variant_builder_add (gvbuilder, (const gchar*)arg_type,
                                   a);
          }
        else if (g_strcmp0 ((const gchar *)arg_type, "d") == 0)
          {
            gdouble a;
            a = g_strtod (http_value, NULL);
            g_variant_builder_add (gvbuilder, (const gchar*)arg_type,
                                   a);
          }
        else if (g_strcmp0 ((const gchar *)arg_type, "s") == 0)
          {
            g_variant_builder_add (gvbuilder, (const gchar*)arg_type,
                                   http_value);
          }
      }


  g_hash_table_destroy (parsed_http_data);


  return g_variant_builder_end (gvbuilder);
}

void
dbus_client_call (DBusClient *dbus_client,
                  const gchar *method,
                  const gchar *data)
{
  GHashTable *arg_info;
  gboolean found_key;
  GError *error = NULL;

  found_key = g_hash_table_lookup_extended (dbus_client->interface,
                                            method,
                                            NULL,
                                            (gpointer)&arg_info);

  /* Return if we don't know about the the method was called */
  if (!found_key)
    return;

  g_debug ("rg_info : %p", arg_info);

  /* No arguments required for this method */
  if (g_hash_table_size (arg_info) == 0)
    g_dbus_proxy_call_sync (dbus_client->proxy, method, NULL, 0, -1, NULL,
                            &error);
  else
    {
      GVariant *arg_data;
      arg_data = parse_http_data (arg_info, data);
      g_dbus_proxy_call_sync (dbus_client->proxy,
                              method,
                              arg_data,
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

static gboolean
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
      return FALSE;
    }

  g_variant_get (result, "(&s)", &xml_introspec_data);

  node_info = g_dbus_node_info_new_for_xml (xml_introspec_data, &error);

  if (error)
    {
      g_critical ("Cannot parse introspection data %s", error->message);
      g_error_free (error);
      return FALSE;
    }

  /* for now we only care about one interface's info */
  info = g_dbus_node_info_lookup_interface (node_info,
                         g_dbus_proxy_get_interface_name (dbus_client->proxy));

  dbus_client->interface = g_hash_table_new_full (g_str_hash,
                                                  g_str_equal,
                                                  g_free,
                                                  (GDestroyNotify)
                                                  g_hash_table_destroy);

  if (!info)
    {
      g_critical ("Bus name does not exist");
      exit (-1);
    }

  g_debug ("interface: %s", info->name);


  for (i=0; (method_info = info->methods[i]); i++)
    {

      GHashTable *arg_info_table = NULL;
      g_debug (" - %s", method_info->name);
      arg_info_table = g_hash_table_new (g_str_hash, g_str_equal);
      if (method_info->in_args)
        {
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
  return TRUE;
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
        if (!dbus_client_inspect (dbus_client))
          {
            /* Can't inspect so bailing out */
            g_object_unref (dbus_client->connection);
            g_object_unref (dbus_client->proxy);
            g_free (dbus_client);
            return NULL;
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

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
 * along with this program; if not, see <http://www.gnu.org/licenses>
 *
 * Written by: Michael Wood <michael.g.wood@linux.intel.com>
 */

#include <stdlib.h>
#include <glib.h>
#include <string.h>
#include <libsoup/soup.h>

#include "http-server.h"

static void
http_post_send_response (SoupMessage *msg)
{
  soup_message_set_status (msg, SOUP_STATUS_OK);
}

static void
headers_ispct (const gchar *name, const gchar *value, gpointer bla)
{
  g_debug ("name: %s value: %s", name, value);
}

static void
http_post_handler (SoupMessage *msg, const gchar *path,
                   DBusClient *dbus_client)
{
  gchar *data;
  gchar *new_path;

  new_path = g_strdup (path + strlen ("/"));

  if (msg->method == SOUP_METHOD_POST)
      data = g_strdup (msg->request_body->data);
  else
    {
      gchar *tmp;
      tmp = soup_uri_to_string (soup_message_get_uri (msg), TRUE);
      data = g_strdup (tmp + strlen ("/?") + strlen (new_path));
      g_free (tmp);
    }

  g_debug ("got post %s", path + strlen ("/"));

  soup_message_headers_foreach (msg->request_headers, headers_ispct, NULL);
  g_debug ("data: %s", data);

  dbus_client_call (dbus_client, path + strlen ("/"), data);

  g_free (data);
  g_free (new_path);

  http_post_send_response (msg);
}


static void
server_cb (SoupServer        *server,
           SoupMessage       *msg,
           const char        *path,
           GHashTable        *query,
           SoupClientContext *client,
           DBusClient        *dbus_client)
{
  g_debug ("%s %s HTTP/1.%d", msg->method, path, soup_message_get_http_version (msg));

  if (msg->method == SOUP_METHOD_POST || msg->method == SOUP_METHOD_GET || msg->method == SOUP_METHOD_HEAD)
    http_post_handler (msg, path, dbus_client); /*
  else if (msg->method == SOUP_METHOD_GET || msg->method == SOUP_METHOD_HEAD)
    send_response (server, msg, path, self, NORMAL);*/
  else
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
}

void
start_http_server (DBusClient *dbus_client)
{
  SoupServer *server;

  server = soup_server_new (SOUP_SERVER_PORT, 9999,
                           SOUP_SERVER_SERVER_HEADER, "httpdbusd",
                           NULL);

  if (!server)
    g_error ("Sorry, unable to start server");

  soup_server_run_async (server);

  soup_server_add_handler (server, NULL, (SoupServerCallback)server_cb,
                           dbus_client,
                           NULL);
}



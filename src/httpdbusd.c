/*
 * httpdbusd - a dbus http bridge daemon
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

#include "dbus-client.h"
#include "http-server.h"

int main (int argc, char **argv)
{
  GOptionContext *context;

  GMainLoop *main_loop;
  gchar *opt_bus_name, *opt_path, *opt_interface;
  GError *error=NULL;
  DBusClient *dbus_client;

  GOptionEntry entries[] =
    {
        { "bus-name", 'b', 0, G_OPTION_ARG_STRING, &opt_bus_name,
          "Bus name to use", NULL },
        { "object-path", 'o', 0, G_OPTION_ARG_STRING, &opt_path,
          "Object path", NULL },
        { "interface", 'i', 0, G_OPTION_ARG_STRING, &opt_interface,
          "Interface to use", NULL},
        { NULL }
    };

  g_type_init ();

  context = g_option_context_new ("- A http dbus bridge daemon");

  g_option_context_add_main_entries (context, entries, "httpdbusd");

  if (!g_option_context_parse (context, &argc, &argv, &error))
    g_warning ("Failed to parse options: %s", error->message);


  dbus_client = dbus_client_new (opt_bus_name, opt_path, opt_interface);

  start_http_server (dbus_client);

  main_loop = g_main_loop_new (NULL, TRUE);

  g_main_loop_run (main_loop);

  if (context)
    g_option_context_free (context);
  if (dbus_client)
    dbus_client_free (dbus_client);

  return EXIT_SUCCESS;
}


     

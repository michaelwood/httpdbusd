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

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#include <glib.h>
#include <gio/gio.h>
#include "dbus-client.h"

G_BEGIN_DECLS

/*typedef struct _DBusClient DBusClient;

struct _DBusClient
{
  GDBusConnection *connection;
  GDBusProxy *proxy;
  GHashTable *interface;
};
*/

void start_http_server (DBusClient *dbus_client, guint port);

G_END_DECLS

#endif

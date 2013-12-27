/*
 * npw-use-tcp-sockets.c
 *
 *
 * Note: Problem in compiling npw-remote-agent-info as 64bits... in 32bit
 *
 * 21/8/2010 - Nito@Qindel.ES
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "sysdeps.h"
#include "npw-use-tcp-sockets.h"
#define DEBUG 1
#include "debug.h"

static int use_tcp_sockets_flag = 0;
static int use_remote_invocation_flag = 0;

int use_tcp_sockets() {
  return use_tcp_sockets_flag;
}

int set_tcp_sockets() {
  use_tcp_sockets_flag = 1;
  return use_tcp_sockets_flag;
}

int unset_tcp_sockets() {
  use_tcp_sockets_flag = 0;
  return use_tcp_sockets_flag;
}

int use_remote_invocation() {
  return use_remote_invocation_flag;
}

int set_remote_invocation() {
  use_remote_invocation_flag = 1;
  set_tcp_sockets();
  return use_remote_invocation_flag;
}

int unset_remote_invocation() {
  use_remote_invocation_flag = 0;
  return use_remote_invocation_flag;
}


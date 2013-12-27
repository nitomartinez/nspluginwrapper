/*
 * npw-agent-info.h
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
#ifndef NPW_AGENT_INFO_H
#define NPW_AGENT_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

// Use TCP/IP communication
//#define NPW_RPC_USE_TCPIP 1
#define NPW_RPC_DEFAULT_PORT 2111
//#define NPW_RPC_TEST_PORT2 3111
#define NPW_RPC_DEFAULT_SERVER "127.0.0.1"
//#define NPW_RPC_TEST_SERVER2 "127.0.0.1"
#define MAX_CONNECTION_PATH 128
  // define This environment var to query the agent
#define NPW_REMOTE_AGENT_ENV "NPW_REMOTE_AGENT_URI"
#define GET_RPC_HOST_PORT "get_rpc_host_port"
#define EXECUTE_REMOTE_PLUGIN "execute_plugin"
#define EXECUTE_REMOTE_PLUGIN_KILL "kill_plugin"  // define the environment
  // define xwindow translation service
      //NPW_XWININFO_TRANSLATE_AGENT_ENV
#define NPW_XWININFO_TRANSLATE_AGENT_ENV "NPW_NX_AGENT_URI"
#define TRANSLATE_WINID_OP "translate"
  // define This environment var to 1 to use TCP sockets
#define NPW_USE_TCPIP_PORT "NPW_USE_TCPIP_PORT"
#define QVD_XPROP_TRANSLATION "_QVD_CLIENT_WID"


extern void set_remote_connection_path(char *connection_path, const char *id);
extern int set_tcp_ip_and_remote_invocation_flags();
extern int invoke_remote_plugin(const char *exec_string, const char *id);
extern int invoke_remote_kill(int pid, const char *id);
extern void *translate_parent_window_id(void *windowid, const char *id);

#ifdef __cplusplus
}
#endif

#endif

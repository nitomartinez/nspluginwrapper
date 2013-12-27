/*
 * use-tcp-sockets.h
 *
 * 23/8/2010 - Nito@Qindel.ES
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
#ifndef NPW_USE_TCP_SOCKETS_H
#define NPW_USE_TCP_SOCKETS_H

#ifdef __cplusplus
extern "C" {
#endif

extern int use_tcp_sockets();
extern int set_tcp_sockets();
extern int unset_tcp_sockets();
extern int use_remote_invocation();
extern int set_remote_invocation();
extern int unset_remote_invocation();

#ifdef __cplusplus
}
#endif

#endif

/*
 *  npw-rpc.h - Remote Procedure Calls (NPAPI specialisation)
 *
 *  nspluginwrapper (C) 2005-2007 Gwenole Beauchesne
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef NPW_RPC_H
#define NPW_RPC_H

// NPAPI methods
enum {
  RPC_METHOD_NP_GET_VALUE = 1,
  RPC_METHOD_NP_GET_MIME_DESCRIPTION,
  RPC_METHOD_NP_INITIALIZE,
  RPC_METHOD_NP_SHUTDOWN,

  RPC_METHOD_NPN_USER_AGENT,
  RPC_METHOD_NPN_GET_VALUE,
  RPC_METHOD_NPN_GET_URL,
  RPC_METHOD_NPN_GET_URL_NOTIFY,
  RPC_METHOD_NPN_POST_URL,
  RPC_METHOD_NPN_POST_URL_NOTIFY,
  RPC_METHOD_NPN_STATUS,
  RPC_METHOD_NPN_PRINT_DATA,
  RPC_METHOD_NPN_REQUEST_READ,

  RPC_METHOD_NPP_NEW,
  RPC_METHOD_NPP_DESTROY,
  RPC_METHOD_NPP_SET_WINDOW,
  RPC_METHOD_NPP_GET_VALUE,
  RPC_METHOD_NPP_URL_NOTIFY,
  RPC_METHOD_NPP_NEW_STREAM,
  RPC_METHOD_NPP_DESTROY_STREAM,
  RPC_METHOD_NPP_WRITE_READY,
  RPC_METHOD_NPP_WRITE,
  RPC_METHOD_NPP_STREAM_AS_FILE,
  RPC_METHOD_NPP_PRINT,

  RPC_METHOD_NPN_CREATE_OBJECT,
  RPC_METHOD_NPN_RETAIN_OBJECT,
  RPC_METHOD_NPN_RELEASE_OBJECT,
  RPC_METHOD_NPN_INVOKE,
  RPC_METHOD_NPN_INVOKE_DEFAULT,
  RPC_METHOD_NPN_EVALUATE,
  RPC_METHOD_NPN_GET_PROPERTY,
  RPC_METHOD_NPN_SET_PROPERTY,
  RPC_METHOD_NPN_REMOVE_PROPERTY,
  RPC_METHOD_NPN_HAS_PROPERTY,
  RPC_METHOD_NPN_HAS_METHOD,
  RPC_METHOD_NPN_SET_EXCEPTION,

  RPC_METHOD_NPN_GET_STRING_IDENTIFIER,
  RPC_METHOD_NPN_GET_STRING_IDENTIFIERS,
  RPC_METHOD_NPN_GET_INT_IDENTIFIER,
  RPC_METHOD_NPN_IDENTIFIER_IS_STRING,
  RPC_METHOD_NPN_UTF8_FROM_IDENTIFIER,
  RPC_METHOD_NPN_INT_FROM_IDENTIFIER,

  RPC_METHOD_NPCLASS_INVALIDATE,
  RPC_METHOD_NPCLASS_HAS_METHOD,
  RPC_METHOD_NPCLASS_INVOKE,
  RPC_METHOD_NPCLASS_INVOKE_DEFAULT,
  RPC_METHOD_NPCLASS_HAS_PROPERTY,
  RPC_METHOD_NPCLASS_GET_PROPERTY,
  RPC_METHOD_NPCLASS_SET_PROPERTY,
  RPC_METHOD_NPCLASS_REMOVE_PROPERTY,
};

// NPAPI data types
enum {
  RPC_TYPE_NPP = 1,
  RPC_TYPE_NP_STREAM,
  RPC_TYPE_NP_BYTE_RANGE,
  RPC_TYPE_NP_SAVED_DATA,
  RPC_TYPE_NP_NOTIFY_DATA,
  RPC_TYPE_NP_RECT,
  RPC_TYPE_NP_WINDOW,
  RPC_TYPE_NP_PRINT,
  RPC_TYPE_NP_FULL_PRINT,
  RPC_TYPE_NP_EMBED_PRINT,
  RPC_TYPE_NP_PRINT_DATA,
  RPC_TYPE_NP_OBJECT,
  RPC_TYPE_NP_IDENTIFIER,
  RPC_TYPE_NP_STRING,
  RPC_TYPE_NP_VARIANT,
};

// NPPrintData is used to get the plugin printed tmpfile
typedef struct _NPPrintData {
  uint32_t size;
  uint8_t data[4096];
} NPPrintData;

// Initialize marshalers for NS4 plugin types
extern int rpc_add_np_marshalers(void) attribute_hidden;

// RPC types
extern int rpc_type_of_NPNVariable(int variable) attribute_hidden;
extern int rpc_type_of_NPPVariable(int variable) attribute_hidden;

#endif /* NPW_RPC_H */

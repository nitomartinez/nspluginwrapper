/*
 * npw-remote-agent-info.c
 *
 * We need to use libcurl to fetch a URL for the plugin, because
 * the initialization of the RPC channel is before the plugin is initialized
 * and we don't have the NPP instance for NPP_GetURL (or similar funcs)
 *
 * Description:
 *
 * If NPW_USE_TCPIP is defined then we use TCP sockets instead of UNIX sockets
 *
 * If NPW_REMOTE_AGENT_URI is defined
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
/*#include <curl/types.h> */
#include <curl/easy.h>
#include <X11/Xlib.h>
#include "npw-use-tcp-sockets.h"
#include "npw-remote-agent-info.h"

#define DEBUG 1
#include "debug.h"

//#define MYURL "http://localhost/port"
#define USERAGENT "QVDnswrapper/1.0"
#define MAXKEYS 128
#define MAX_KEY_SIZE 128
#define MAX_VALUE_SIZE 128
#define MAX_URL_SIZE 1024
struct MemoryStruct {
  char *memory;
  size_t size;
};


static int rpc_port = 0;
char rpc_host[MAX_URL_SIZE];
char *npw_agent_uri = NULL;
char *npw_xwin_translate_service_uri = NULL;



/******************************************************************************
 * This is a replacement for strsep which is not portable (missing on
 Solaris).
 */
 static char* getToken(char** str, const char* delims)
 {
     char* token;

     if (*str==NULL) {
         /* No more tokens */
         return NULL;
     }

     token=*str;
     while (**str!='\0') {
         if (strchr(delims,**str)!=NULL) {
             **str='\0';
             (*str)++;
             return token;
         }
         (*str)++;
     }
     /* There is no other token */
     *str=NULL;
     return token;
 }


static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
     NULL pointers, so we take care of it here */
  if(ptr)
      return realloc(ptr, size);
  else
      return malloc(size);
}


static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

void init_array(char **array) {
  memset(array, 0, MAXKEYS * sizeof(char *));
}

void free_array(char **array) {
  int i;
  char **ptr;
  ptr=array;
  for (i = 0; i < MAXKEYS; ++i) {
    if (*ptr) {
      free(*ptr);
      *ptr=NULL;
    }
    ++ ptr;
  }
}

void assign(char **ptr, const char *value) {
  int len;
  len = strlen(value);
  *ptr = malloc(len + 1);
  strncpy(*ptr, value, len+1);
}

//Duplicate entries ocupy space, but only the first is used
int parse_page(struct MemoryStruct *chunk, char **keys, char **values) {
  char *line, *copy, **copy_ptr, *copy_free_ptr, format[32],
    key[MAX_KEY_SIZE + 1], value[MAX_VALUE_SIZE + 1];
  int parsed_elements, keys_parsed;

  D(bug("parse_page: start\n"));
  // Format string to parse chars
  sprintf(format, " %%%d[^= ] = %%%d[^#] ", MAX_KEY_SIZE, MAX_VALUE_SIZE);

  // Set end of string for long keys and values
  key[MAX_KEY_SIZE] = 0;
  value[MAX_VALUE_SIZE] = 0 ;

  // Copy the buffer to be parsed
  copy_free_ptr = copy = malloc(chunk->size);
  memcpy(copy, chunk->memory, chunk->size);
  copy_ptr = &copy;


  // Parse line by line
  line = getToken(copy_ptr, "\n");
  for (keys_parsed=0; *copy_ptr != NULL; line = getToken(copy_ptr, "\n")) {
    D(bug("parse_page: line %s\n", line));
    if (line == NULL)       // Should never happen
      break;

    if (keys_parsed >= MAXKEYS) {
      	npw_printf("ERROR: Too many keys in the file. Skipping the rest\n");
      break;
    }

    if (strlen(line) == 0)
      continue;

    if (line[0] == '#')
      continue;

    if ((parsed_elements = sscanf(line, format, key, value)) < 2) {
	npw_printf("ERROR: line <%s> not recognized key=%s, value=%s, int=%d\n", line, key, value, parsed_elements);
      continue;
    }

    D(bug("parse_page: key %s value %s\n", key, value));
    assign(keys, key);
    keys ++;
    assign(values, value);
    values ++;
    keys_parsed ++;
  }
  free(copy_free_ptr);
  return 0;
}

// Not defined and empty is the same
const char * getvalue(char **keys, char**values, const char *key) {
  int i;
  for(i=0; i < MAXKEYS; i++) {
    if ((*keys == NULL) || (*values == NULL)) {
      break;
    }
    if (!strcmp(key, *keys)){
      return *values;
    }
    keys ++;
    values ++;
  }
  return "";
}


int fetch_remote_agent_url(const char *myurl, struct MemoryStruct *chunk) {
  CURL *handle;
  CURLcode res;
  char error_buffer[CURL_ERROR_SIZE];



  if (curl_global_init(CURL_GLOBAL_ALL)) {
    npw_printf("Error intializing curl for url %s", myurl);
    return -1;
  }

  handle = curl_easy_init();
  curl_easy_setopt(handle, CURLOPT_URL, myurl);
  curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
  curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)chunk);
  curl_easy_setopt(handle, CURLOPT_USERAGENT, USERAGENT);
  curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, error_buffer);
  res = curl_easy_perform(handle);
  if (res) {
    npw_printf("Error getting URL %s: %s", myurl, error_buffer);
  }
  curl_easy_cleanup(handle);
  curl_global_cleanup();
  return res;
}



int return_rpc_port() {
  return rpc_port++;
}

int set_tcp_ip_and_remote_invocation_flags() {
  //  int use_tcp = 0;
  // If NPW_USE_TCPIP_PORT or NPW_REMOTE_AGENT_ENV is defined then use tcp_sockets
  const char *npw_tcp_port = getenv(NPW_USE_TCPIP_PORT);
  if (npw_tcp_port)
    {
      set_tcp_sockets();
      rpc_port = atoi(npw_tcp_port);
      if (rpc_port >= 1024 && rpc_port <= 65535)
	{
	  D(bug("%s defined: %d. Using tcp sockets\n", NPW_USE_TCPIP_PORT, rpc_port));
	}
      else
	{
	  D(bug("%s defined: %s. But the socket is < 1024 or > 65535. Setting default %d\n", NPW_USE_TCPIP_PORT, npw_tcp_port, rpc_port));
	  rpc_port = NPW_RPC_DEFAULT_PORT;
	}
    }
  npw_agent_uri = getenv(NPW_REMOTE_AGENT_ENV);
  if (npw_agent_uri)
    {
      set_tcp_sockets();
      set_remote_invocation();
      D(bug("verify_if_using_tcp_sockets %s defined: %s. Using tcp sockets\n", NPW_REMOTE_AGENT_ENV, npw_agent_uri));
    }
  else
    {
      D(bug("verify_if_using_tcp_sockets checking for http url %s not defined\n", NPW_REMOTE_AGENT_ENV));
    }
  return use_tcp_sockets();
}

void dump_keys(char **keys, char **values) {
  int i;

  for (i=0; (i<MAXKEYS && *keys != NULL && *values != NULL); ++i) {
    D(bug("dump_keys: key %d, %s, %s\n", i, *keys, *values));
    keys ++;
    values ++;
  }

}

void init_mem(char **keys, char **values,   struct MemoryStruct *chunk) {
  init_array(keys);
  init_array(values);
  chunk->memory=NULL;
  chunk->size=0;
}

void free_mem(char **keys, char **values,   struct MemoryStruct *chunk) {
  free_array(keys);
  free_array(values);
  if (chunk->memory)
    free(chunk->memory);
  chunk->size=0;
}

void build_url(char *myurl, int size, const char *baseurl, const char *op, const char *id) {
  if (npw_agent_uri[strlen(npw_agent_uri) - 1] == '/')
    snprintf(myurl, size, "%s%s?%s", baseurl, op, id);
  else
    snprintf(myurl, size, "%s/%s?%s", baseurl, op, id);

  myurl[size] = 0;
}

void set_remote_connection_path(char *connection_path, const char *id) {
  int ret_code;
  struct MemoryStruct chunk;
  char *keys[MAXKEYS];
  char *values[MAXKEYS];
  const char *host;
  const char *port;
  char myurl[MAX_URL_SIZE];
  char parameters[MAX_URL_SIZE];

  init_mem(keys, values, &chunk);

  if (!npw_agent_uri)  // Using TCP but no remote HTTP agent
    {
      D(bug("set_remote_connection_path: npw_agent_uri is null\n"));
      sprintf(connection_path, "%s:%d", NPW_RPC_DEFAULT_SERVER, return_rpc_port());
      return;
    }

  D(bug("set_remote_connection_path from URL %s with id %s\n", npw_agent_uri, id));
  snprintf(parameters, MAX_URL_SIZE, "id=%s", id);
  build_url(myurl, MAX_URL_SIZE, npw_agent_uri, GET_RPC_HOST_PORT, parameters);

  D(bug("set_remote_connection_path my url is %s\n", myurl));

  ret_code = fetch_remote_agent_url(myurl, &chunk);
  if (ret_code) {
    npw_printf("ERROR: Error fetching url %s\n", npw_agent_uri);
    return;
  }

  parse_page(&chunk, keys, values);
  D(dump_keys(keys, values));

  host = getvalue(keys, values, "host");
  port = getvalue(keys, values, "port");

  if (strlen(host) == 0) {
    host = NPW_RPC_DEFAULT_SERVER;
    npw_printf("ERROR: Error fetching key host from url %s. Setting to %s\n", npw_agent_uri, NPW_RPC_DEFAULT_SERVER);
  }
  if (strlen(port) == 0) {
    port = "2111";
    npw_printf("ERROR: Error fetching key port from url %s. Setting to %d\n", npw_agent_uri, NPW_RPC_DEFAULT_PORT);
  }
  sprintf(connection_path, "%s:%s", host, port);
  D(bug("set_remote_connection_path from URL %s with connection_path %s\n", npw_agent_uri, connection_path));

  free_mem(keys, values, &chunk);

  return;
}

// Returns the remote pid, -1 if error
// Parameters passed are the id and the execute string
int invoke_remote_plugin(const char *exec_string, const char *id) {
  int pid = -1, ret_code;
  struct MemoryStruct chunk;
  char *keys[MAXKEYS];
  char *values[MAXKEYS];
  const char *pid_str;
  char myurl[MAX_URL_SIZE];
  char parameters[MAX_URL_SIZE];

  init_mem(keys, values, &chunk);

  if (!npw_agent_uri)  // Using TCP but no remote HTTP agent
    {
      npw_printf("ERROR: invoke_remote_plugin no remote URL should never happen\n");
      return -1;
    }

  D(bug("invoke_remote_plugin from URL %s with id %s\n", npw_agent_uri, id));
  snprintf(parameters, MAX_URL_SIZE, "exec=%s&id=%s", exec_string, id);
  build_url(myurl, MAX_URL_SIZE, npw_agent_uri, EXECUTE_REMOTE_PLUGIN, parameters);

  D(bug("invoke_remote_plugin url is <%s>\n", myurl));

  ret_code = fetch_remote_agent_url(myurl, &chunk);
  if (ret_code) {
    npw_printf("ERROR: Error fetching url %s\n", npw_agent_uri);
    return -1;
  }

  parse_page(&chunk, keys, values);
  D(dump_keys(keys, values));

  pid_str = getvalue(keys, values, "pid");
  pid = atoi (pid_str);

  free_mem(keys, values, &chunk);
  return pid;
}

int invoke_remote_kill(int pid, const char *id) {
  int ret_code;
  struct MemoryStruct chunk;
  char *keys[MAXKEYS];
  char *values[MAXKEYS];
  const char *pid_str;
  char myurl[MAX_URL_SIZE];
  char parameters[MAX_URL_SIZE];

  init_mem(keys, values, &chunk);

  if (!npw_agent_uri)  // Using TCP but no remote HTTP agent
    {
      npw_printf("ERROR: invoke_remote_plugin no remote URL should never happen\n");
      return -1;
    }

  D(bug("invoke_remote_kill from URL %s with id %s and pid %d\n", npw_agent_uri, id, pid));
  snprintf(parameters, MAX_URL_SIZE, "pid=%d&id=%s", pid, id);
  build_url(myurl, MAX_URL_SIZE, npw_agent_uri, EXECUTE_REMOTE_PLUGIN_KILL, parameters);

  D(bug("invoke_remote_plugin url is <%s>\n", myurl));

  ret_code = fetch_remote_agent_url(myurl, &chunk);
  if (ret_code) {
    npw_printf("ERROR: Error fetching url %s\n", npw_agent_uri);
    return -1;
  }

  parse_page(&chunk, keys, values);
  D(dump_keys(keys, values));

  pid_str = getvalue(keys, values, "pid");
  pid = atoi (pid_str);

  free_mem(keys, values, &chunk);
  return pid;
}

Window getChild_r(Window windowid, Display *display, unsigned int width, unsigned int height)
{
  Window *children, root_return, parent_return, *children_ptr, child;
  unsigned int nchildren_return, i, border_width, depth,
    children_width, children_height;
  int x, y;


  Status result = XQueryTree(display,
			     windowid,
			     &root_return,
			     &parent_return,
			     &children,
			     &nchildren_return);
  if (!result)
    {
      npw_printf("Error getting XQueryTree for id 0x%lx\n", windowid);
      return (0);
    }

  printf("Num children for window id 0x%lx (%ux%u) : %d\n", windowid, width, height, nchildren_return);

  for (i=0, children_ptr = children ; i < nchildren_return ; i ++)
    {
      printf("Child id: 0x%lx\n", *children_ptr);
      result = XGetGeometry(display, *children_ptr, &root_return, &x, &y,
			    &children_width, &children_height, &border_width,
			    &depth);
      if (!result)
	{
	  npw_printf("Error getting XGetGeometry for id 0x%lx\n", windowid);
	  return (0);
	}

      if (children_width == width && children_height == height)
	{
	  printf("Child 0x%lx has the same height and width ast the parent. Traversing tree\n", *children_ptr);
	  child = getChild_r(*children_ptr, display, width, height);
	  XFree(children);
	  if (child)
	    return child;
	  else
	    return windowid;
	}
      else
	{
	  printf("Skipping child 0x%lx because the height and width are differnt\n", *children_ptr);
	}
      children_ptr ++;
    }

  XFree(children);
  // Not found;
  return windowid;
}

/*
 * If no child exists of the same size it returns the same windowid
 * It returns 0 on error
 */
Window getChild(Window windowid)
{
  Display  *display;
  Window root_return;
  unsigned int width, height, border_width, depth;
  int x, y;

  display = XOpenDisplay(0);
  if (!display)
    {
      npw_printf("can't open display\n");
      return (0);
    }


  Status result = XGetGeometry(display, windowid, &root_return, &x, &y,
			&width, &height, &border_width, &depth);
  if (!result)
    {
      npw_printf("Error getting XGetGeometry for id 0x%lx\n", windowid);
      return (0);
    }

  return getChild_r(windowid, display, width, height);
}

Window getWindowidProp(Window windowid)
{
  const char *property = QVD_XPROP_TRANSLATION;
  Atom     actual_type;
  int      actual_format;
  unsigned long  nitems;
  unsigned long  bytes;
  long     *data = NULL;

  Display *display = XOpenDisplay(0);
  if (!display)
    {
      npw_printf("can't open display\n");
      return (0);
    }

  D(bug("getWindowidProp, trying to get property: %s\n", property));
  Atom propertyAtom = XInternAtom(display, property, True);
  D(bug("getWindowidProp, after getting property: %s, %ld\n", property, propertyAtom));
  if (propertyAtom == None)
    {
      npw_printf("The Xwindows property %s does not exists\n", property);
      return 0;
    }
  Status result = XGetWindowProperty(
                display,
                windowid,
                propertyAtom, //replace this with your property
                0,
                (~0L),
                False,
                AnyPropertyType,
                &actual_type,
                &actual_format,
                &nitems,
                &bytes,
                (unsigned char**)&data);
  if (result != Success)
    {
      npw_printf("Error getting XGetWindowProperty %s for id 0x%lx\n", property, windowid);
      return (0);
    }
  D(bug ("Data actual_type: %ld, format: %d, nitems: %ld, bytes: %ld, data: 0x%lx %ld\n", actual_type, actual_format, nitems, bytes, *data, *data));
  if (nitems < 1 || data == NULL)
    return 0;
  return *data;
}



/*
 * Receives as input the window id, and returns the same or the translated one
 * On error the same windowid is returned. is this correct? (TODO)
 */

/* void *translate_parent_window_id(void *windowid, const char *id) */
/* { */
/*   Window new_windowid = getWindowidProp((Window)windowid); */

/*   if (new_windowid == 0) */
/*     { */
/*       new_windowid = (Window)windowid; */
/*       npw_printf("translate_parent_window_id returned null, setting the same windowid %p\n", (void *)new_windowid); */
/*     } */

/*   D(bug("translate_parent_window_id for id %s translated windowid for from %p->%p [%lx]\n", id, windowid, (void *)new_windowid, new_windowid)); */
/*   return (void *)new_windowid; */
/* } */

void *translate_parent_window_id(void *windowid, const char *id)
{
  int ret_code;
  struct MemoryStruct chunk;
  char *keys[MAXKEYS];
  char *values[MAXKEYS];
  const char *windowid_str;
  char myurl[MAX_URL_SIZE];
  char parameters[MAX_URL_SIZE];
  void *new_windowid;

  npw_xwin_translate_service_uri = getenv(NPW_XWININFO_TRANSLATE_AGENT_ENV);
  if (!npw_xwin_translate_service_uri) {
      D(bug("translate_parent_window_id: not defined translation service env var: %s for id %s. Returning the same windowid %p", NPW_XWININFO_TRANSLATE_AGENT_ENV, id, windowid));
      return windowid;
  }

  D(bug("translate_parent_window_id env var %s defined for id %s: %s. windowid=%p\n", NPW_XWININFO_TRANSLATE_AGENT_ENV, id, npw_xwin_translate_service_uri, windowid));

  init_mem(keys, values, &chunk);
  snprintf(parameters, MAX_URL_SIZE, "windowid=%p&id=%s", windowid, id);
  build_url(myurl, MAX_URL_SIZE, npw_xwin_translate_service_uri, TRANSLATE_WINID_OP, parameters);

  D(bug("translate_parent_window_id invoke_remote_plugin url is <%s>\n", myurl));
  ret_code = fetch_remote_agent_url(myurl, &chunk);
  if (ret_code) {
    npw_printf("ERROR: Error fetching url %s\n", npw_agent_uri);
    return windowid;
  }

  parse_page(&chunk, keys, values);
  D(dump_keys(keys, values));
  windowid_str = getvalue(keys, values, "windowid");
  if (sscanf(windowid_str, "%p", &new_windowid) < 1) {
    npw_printf("ERROR: translate_parent_window_id Unable to parse new windowid <%s>\n", windowid_str);
    return windowid;
  }

  D(bug("translate_parent_window_id translated windowid for id %s from %p->%p\n", id, windowid, new_windowid));
  return new_windowid;

}


#include <EndpointSecurity/EndpointSecurity.h>
#include <stdio.h>
#include <string.h>

es_handler_block_t handler =  ^(es_client_t *c, const es_message_t *msg ) {
  const char* path = msg->event.open.file->path.data;
  size_t path_size = msg->event.open.file->path.length;
  es_respond_result_t res;
  
  printf("Event: %d, Process %s, used the OPEN system call and tried to open file %s\n", msg->event_type, msg->process->executable->path.data, path); 
  
  if(strncmp(path, "/etc/passwd", path_size) == 0 ) {
    res = es_respond_flags_result(c, msg, 0, true);
  } else {
    // documentation specifies we need to response with UINT32_MAX to allow the event 
    res = es_respond_flags_result(c, msg, UINT32_MAX, true);
  }
  
  switch(res) {
    case ES_RESPOND_RESULT_ERR_EVENT_TYPE:
      printf("ES RESPONSE: wrong event typE\n");
      break;
    case ES_RESPOND_RESULT_ERR_INVALID_ARGUMENT:
      printf("ES RESPONSE: wrong arguments\n");
      break;
    case ES_RESPOND_RESULT_ERR_DUPLICATE_RESPONSE:
      printf("ES RESPONSE: message was already responded\n");
      break;
    case ES_RESPOND_RESULT_NOT_FOUND:
      printf("ES RESPONSE: message not found\n");
      break;
    case ES_RESPOND_RESULT_ERR_INTERNAL:
      printf("ES RESPONSE: error communicating with the subsystem\n");
      break;
    case ES_RESPOND_RESULT_SUCCESS:
      printf("ES RESPONSE: successfully responded to message\n");
      break;
    default:
      break;
  }
};

int main(int argc, char** argv) {
  es_client_t *g_client;
  es_new_client_result_t endpoint_security = es_new_client(&g_client, handler);

  if(endpoint_security != ES_NEW_CLIENT_RESULT_SUCCESS) {
    printf("Error creating es_new_client\n");
  }
  
  es_event_type_t events[] = {ES_EVENT_TYPE_AUTH_OPEN}; //ES_EVENT_TYPE_NOTIFY_EXEC
  es_return_t subscribed = es_subscribe(g_client, events, sizeof events / sizeof *events);
  
  if(subscribed != ES_RETURN_SUCCESS) {
    printf("Error subscribing es_new_client\n");
  }

  for(;;) {
  }
}

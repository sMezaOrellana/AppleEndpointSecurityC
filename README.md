# Apple's Endpoint Security Example in C

## Introduction
This repository contains a simple C program that demonstrates the usage of Apple's Endpoint Security framework.
The code sets up an Endpoint Security (ES) client, subscribes to a specific event type (in this case, `ES_EVENT_TYPE_AUTH_OPEN`), and responds to the events by blocking a process if it tries to open the /etc/passwords file.
`ES_EVENT_TYPE_AUTH_OPEN` the documentation specifies this event as follows "An identifier for a process that requests permission from the operating system to open a file."
This example serves as an educational resource for understanding how Endpoint Security works on macOS.

## Prerequisites
Before running this code, ensure that you are working on a macOS system, as Endpoint Security is specific to Apple platforms.
Furthermore, if you want to test this example you need to have SIP disabled. You can always run this in a VM.

## Getting Started
1. Clone the repository to your local machine:

    ```bash
    git clone <repository_url>
    ```

2. Navigate to the project directory:

    ```bash
    cd edrmacos
    ```

3. Compile the code:

    ```bash
    make 
    ```

4. Run the executable:

    ```bash
    ./edrmacos
    ```

## Understanding the Code
### 1. Building
Using the Endpoint Security framework requieres us to link to the EndpointSecurity dynamic library.
The bsm libary is needed for some events.
Furthermore, we need to sign the executable with the `com.apple.developer.endpoint-security.client` entitlement.

```Makefile
CC = clang 
CFLAGS = -std=c99 -Werror -Wno-unused-label -Wno-unused-parameter -Wno-cpp -Wall
OBJ_FILES = edrmacos.o
DYN_LYB = -lbsm -lEndpointSecurity
FRAMEWORKS = -framework Foundation

all: edr codesign

edr: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o edrmacos $(FRAMEWORKS) $(DYN_LYB)

edrmacos.o: edrmacos.c
	$(CC) $(CFLAGS) -c edrmacos.c

codesign:
	codesign --sign - \
    	--entitlements $(shell pwd)/reformatted.entitlements \
    	--deep $(shell pwd)/edrmacos \
    	--force

clean:
	rm *.o
```
### 2. Initializing the Endpoint Security Client
In order to connect to the endpointsecurity subsystem we need to initialize a client of type `es_client_t`.
We do this by using the `es_new_client`. This functione expects an `es_client_t` and a Block. 
A Block is "a non-standard extension added by Apple Inc. to Clang's implementations of the C, C++, and Objective-C programming languages that uses a lambda expression-like syntax to create closures within these languages."

The block is the piece of code that will execute whenever our client recieves a new message from the EndpointSecurity subsystem. 
A message is a top level datatype that encodes information sent from the ES subsystem to its clients. 
Each security event being processed by the ES subsystem will be encoded in an es_message_t.  
A message can be an authorization request or a notification of an event that has already taken place.
In the case of authorization requests we need to respond to them within a set amount of time.
```c
es_client_t *g_client;
es_new_client_result_t endpoint_security = es_new_client(&g_client, ^(es_client_t *c, const es_message_t *msg ) {
    // Handle events and respond accordingly
});
```
Our handling code checks if the path of the file a process is trying to open is `/etc/passwd` if this is the case it blocks the authorization request.
Else we allow any the opening of a file.
```c
^(es_client_t *c, const es_message_t *msg ) {
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
```

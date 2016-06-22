// node-ems.cc
#include <node.h>
#include <tibems/tibems.h>
#include <tibems/emsadmin.h>
#include <pthread.h>
#include <errno.h>
#include "emsconnection.h"
#define THREAD_RETVAL         void*
#define THREAD_OBJ            pthread_t
#define THREAD_CREATE(t,f,a)  pthread_create(&(t),NULL,(f),(a))


namespace nodeems {

using v8::Local;
using v8::Object;


void init(Local<Object> exports, Local<Object> module) {
  EmsConnection::Init(exports);
}

NODE_MODULE(addon, init)

}  // namespace nodeems

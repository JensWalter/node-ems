// emsconnection.h
#ifndef EMSCONNECTION_H
#define EMSCONNECTION_H

#include <node.h>
#include <node_object_wrap.h>
#include <tibems/tibems.h>
#include <tibems/emsadmin.h>
#include <iostream>

namespace nodeems {

class EmsConnection : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

 private:
  explicit EmsConnection(const char* p_server,const char* p_user,const char* p_password);
  ~EmsConnection();

  static const char* V8StringToCString(v8::Local<v8::Value> value);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void sendToDestinationSync(bool useTopic,const v8::FunctionCallbackInfo<v8::Value>& args);
  static void sendToQueueSync(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void sendToTopicSync(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void requestFromDestinationSync(bool useTopic,const v8::FunctionCallbackInfo<v8::Value>& args);
  static void requestFromQueueSync(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void requestFromTopicSync(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  const char* server;
  const char* user;
  const char* password;
};

}  // namespace nodeems

#endif
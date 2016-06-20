// node-ems.cc
#include <node.h>
#include <tibems/tibems.h>
#include <tibems/emsadmin.h>
#include <pthread.h>
#include <errno.h>
#define THREAD_RETVAL         void*
#define THREAD_OBJ            pthread_t
#define THREAD_CREATE(t,f,a)  pthread_create(&(t),NULL,(f),(a))


namespace nodeems {

using v8::Exception;
using v8::Number;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

const char* ToCString(const String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

void sendMsgToDestinationSync(bool useTopic,const FunctionCallbackInfo<Value>& args) {
	//sendMsgToQueueSync(server,user,password,queueName,header,body)
  Isolate* isolate = args.GetIsolate();

  // Check the number of arguments passed.
  if (args.Length() != 6) {
    // Throw an Error that is passed back to JavaScript
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }

  tibems_status status = TIBEMS_OK;
  const char* str    = NULL;
  tibemsErrorContext errorContext = NULL;
  tibemsConnectionFactory factory      = NULL;
  tibemsConnection connection   = NULL;
  tibemsSession session      = NULL;
  tibemsMsgProducer msgProducer  = NULL;
  tibemsDestination destination  = NULL;

  status = tibemsErrorContext_Create(&errorContext);
  if (status != TIBEMS_OK){
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, tibemsStatus_GetText(status))));
    return;
  }

  factory = tibemsConnectionFactory_Create();
  if (!factory){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  String::Utf8Value tempstr(args[0]);
  const char * serverUrl = nodeems::ToCString(tempstr);
  status = tibemsConnectionFactory_SetServerURL(factory, serverUrl);
  if (status != TIBEMS_OK) {
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  String::Utf8Value tempstr2(args[1]);
  const char * userName = nodeems::ToCString(tempstr2);
  String::Utf8Value tempstr3(args[2]);
  const char * password = nodeems::ToCString(tempstr3);
  status = tibemsConnectionFactory_CreateConnection(factory,&connection, userName,password);
  if (status != TIBEMS_OK){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  String::Utf8Value tempstr4(args[3]);
  const char * destName = nodeems::ToCString(tempstr4);
  if (useTopic){
    status = tibemsTopic_Create(&destination,destName);
  }else{
    status = tibemsQueue_Create(&destination,destName);
  }
  if (status != TIBEMS_OK){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* create the session */
  status = tibemsConnection_CreateSession(connection, &session,TIBEMS_FALSE,TIBEMS_AUTO_ACKNOWLEDGE);
  if (status != TIBEMS_OK){
	tibemsErrorContext_GetLastErrorString(errorContext, &str);
	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* create the producer */
  status = tibemsSession_CreateProducer(session,&msgProducer,destination);
  if (status != TIBEMS_OK){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  tibemsTextMsg msg =NULL;
  status = tibemsTextMsg_Create(&msg);
  if (status != TIBEMS_OK){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }
        
  /* set the message text */
  String::Utf8Value tempstr5(args[5]);
  const char * body = nodeems::ToCString(tempstr5);
  status = tibemsTextMsg_SetText(msg,body);
  if (status != TIBEMS_OK){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* publish the message */
  status = tibemsMsgProducer_Send(msgProducer,msg);
  if (status != TIBEMS_OK){
  	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }
        
  //fetch message id
  const char* msgId = NULL;
  tibemsMsg_GetMessageID(msg,&msgId);
  Local<v8::String> msgIdTyped = String::NewFromUtf8(isolate, msgId);

  /* destroy the message */
  status = tibemsMsg_Destroy(msg);
  if (status != TIBEMS_OK){
   	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* destroy the destination */
  status = tibemsDestination_Destroy(destination);
  if (status != TIBEMS_OK){
   	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* close the connection */
  status = tibemsConnection_Close(connection);
  if (status != TIBEMS_OK){
   	tibemsErrorContext_GetLastErrorString(errorContext, &str);
  	isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  tibemsErrorContext_Close(errorContext);
  args.GetReturnValue().Set(msgIdTyped);
}


void sendMsgToQueueSync(const FunctionCallbackInfo<Value>& args){
  nodeems::sendMsgToDestinationSync(false,args);
}
void sendMsgToTopicSync(const FunctionCallbackInfo<Value>& args){
  nodeems::sendMsgToDestinationSync(true,args);
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "sendMsgToQueueSync", sendMsgToQueueSync);
  NODE_SET_METHOD(exports, "sendMsgToTopicSync", sendMsgToTopicSync);
}

NODE_MODULE(addon, init)

}  // namespace nodeems

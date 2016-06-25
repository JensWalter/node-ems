// emsconnection.cc
#include "emsconnection.h"

namespace nodeems {

using v8::Context;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Array;

Persistent<Function> EmsConnection::constructor;

EmsConnection::EmsConnection(const char* p_server,const char* p_user,const char* p_password) {
  server = p_server;
  user = p_user;
  password = p_password;
}

EmsConnection::~EmsConnection() {
}

const char* EmsConnection::V8StringToCString(v8::Local<v8::Value> value) {
  v8::String::Utf8Value v8Str(value); 
  char *cStr = (char*) malloc(strlen(*v8Str) + 1); 
  strcpy(cStr, *v8Str); 
  return cStr;
}

void EmsConnection::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "EmsConnection"));
  tpl->InstanceTemplate()->SetInternalFieldCount(3);

  // Prototype
  NODE_SET_PROTOTYPE_METHOD(tpl, "sendToQueueSync", sendToQueueSync);
  NODE_SET_PROTOTYPE_METHOD(tpl, "sendToTopicSync", sendToTopicSync);
  NODE_SET_PROTOTYPE_METHOD(tpl, "requestFromQueueSync", requestFromQueueSync);
  NODE_SET_PROTOTYPE_METHOD(tpl, "requestFromTopicSync", requestFromTopicSync);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "prepare"), tpl->GetFunction());
}

void EmsConnection::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    // Invoked as constructor: `new EmsConnection(...)`
    const char* server = args[0]->IsUndefined() ? "" : V8StringToCString(args[0]);
    const char* user = args[1]->IsUndefined() ? "" : V8StringToCString(args[1]);
    const char* password = args[2]->IsUndefined() ? "" : V8StringToCString(args[2]);
    EmsConnection* obj = new EmsConnection(server,user,password);

    args.This()->Set(String::NewFromUtf8(isolate, "server"), args[0]->ToString());
    args.This()->Set(String::NewFromUtf8(isolate, "user"), args[1]->ToString());
    args.This()->Set(String::NewFromUtf8(isolate, "password"), args[2]->ToString());
    
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    // Invoked as plain function `EmsConnection(...)`, turn into construct call.
    const int argc = 3;
    Local<Value> argv[argc] = { args[0],args[1],args[2] };
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Object> result =
        cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(result);
  }
}

void EmsConnection::sendToDestinationSync(bool useTopic,const FunctionCallbackInfo<Value>& args) {
  //sendMsgToQueueSync(queueName,header,body)
  Isolate* isolate = args.GetIsolate();
  EmsConnection* ems = ObjectWrap::Unwrap<EmsConnection>(args.Holder());

  // Check the number of arguments passed.
  if (args.Length() != 3) {
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

  status = tibemsConnectionFactory_SetServerURL(factory, ems->server);
  if (status != TIBEMS_OK) {
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  status = tibemsConnectionFactory_CreateConnection(factory,&connection, ems->user, ems->password);
  if (status != TIBEMS_OK){
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  const char * destName = V8StringToCString(args[0]); 
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
  const char * body = V8StringToCString(args[2]);
  status = tibemsTextMsg_SetText(msg,body);
  if (status != TIBEMS_OK){
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* set header properties */
  if(args[1]->IsObject()){
    Local<Object> lobj = args[1]->ToObject();
    Local<Array> property_names = lobj->GetOwnPropertyNames();
    for (uint32_t i = 0; i < property_names->Length(); ++i) {
        Local<Value> key = property_names->Get(i);
        Local<Value> value = lobj->Get(key);
        if (key->IsString() && value->IsString()) {
          tibemsMsg_SetStringProperty(msg, V8StringToCString(key), V8StringToCString(value));
        }
    }
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

  tibemsMsgProducer_Close(msgProducer);
  tibemsSession_Close(session);

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

void EmsConnection::requestFromDestinationSync(bool useTopic,const FunctionCallbackInfo<Value>& args) {
  //sendMsgToQueueSync(queueName,header,body)
  Isolate* isolate = args.GetIsolate();
  EmsConnection* ems = ObjectWrap::Unwrap<EmsConnection>(args.Holder());

  
  // Check the number of arguments passed.
  if (args.Length() != 3) {
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
  tibemsMsgProducer producer  = NULL;
  tibemsDestination destination  = NULL;
  tibemsTemporaryQueue response_queue = NULL;
  tibemsTemporaryTopic response_topic = NULL;
  tibemsMsgConsumer consumer  = NULL;

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

  status = tibemsConnectionFactory_SetServerURL(factory, ems->server);
  if (status != TIBEMS_OK) {
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  status = tibemsConnectionFactory_CreateConnection(factory,&connection, ems->user, ems->password);
  if (status != TIBEMS_OK){
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  const char * destName = V8StringToCString(args[0]);
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
  status = tibemsSession_CreateProducer(session,&producer,destination);
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
        
  /* create response queue*/
  if (useTopic){
    tibemsSession_CreateTemporaryTopic(session,&response_topic );
    tibemsSession_CreateConsumer(session, &consumer,response_topic,NULL,TIBEMS_FALSE);
  }else{
    tibemsSession_CreateTemporaryQueue(session,&response_queue );
    tibemsSession_CreateConsumer(session, &consumer,response_queue,NULL,TIBEMS_FALSE);
  }
  status = tibemsConnection_Start(connection);
  
  /* set the message text */
  const char * body = V8StringToCString(args[2]);
  status = tibemsTextMsg_SetText(msg,body);
  if (status != TIBEMS_OK){
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* set header properties */
  if(args[1]->IsObject()){
    Local<Object> lobj = args[1]->ToObject();
    Local<Array> property_names = lobj->GetOwnPropertyNames();
    for (uint32_t i = 0; i < property_names->Length(); ++i) {
        Local<Value> key = property_names->Get(i);
        Local<Value> value = lobj->Get(key);
        if (key->IsString() && value->IsString()) {
          tibemsMsg_SetStringProperty(msg, V8StringToCString(key), V8StringToCString(value));
        }
    }
  }

  if (useTopic){
    tibemsMsg_SetReplyTo(msg,response_topic);
  }else{
    tibemsMsg_SetReplyTo(msg,response_queue);
  }

  /* send the message */
  status = tibemsMsgProducer_Send(producer, msg);
  if (status != TIBEMS_OK){
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* listen for response */
  tibemsTextMsg response_msg =NULL;
  status = tibemsMsgConsumer_ReceiveTimeout(consumer,&response_msg,50000);
  if (status == TIBEMS_TIMEOUT){
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "waiting for response timed out")));
    return;
  }

  /* destroy the message */
  status = tibemsMsg_Destroy(msg);
  if (status != TIBEMS_OK){
    tibemsErrorContext_GetLastErrorString(errorContext, &str);
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, str)));
    return;
  }

  /* eval response message */
  Local<Object> obj = Object::New(isolate);
  Local<Object> header = Object::New(isolate);
  tibemsDestination response_destination = NULL;
  tibemsMsg_GetDestination(response_msg, &response_destination);
  char * response_destName = (char*)malloc(100); 
  tibemsDestination_GetName(response_destination,response_destName,100);
  header->Set(String::NewFromUtf8(isolate, "JMSDestination"), String::NewFromUtf8(isolate, response_destName));
  free(response_destName);
  tibemsMsg_GetMessageID(response_msg, &str);
  header->Set(String::NewFromUtf8(isolate, "JMSMessageID"), String::NewFromUtf8(isolate, str));
  tibemsMsg_GetCorrelationID(response_msg, &str);
  header->Set(String::NewFromUtf8(isolate, "JMSCorrelationID"), String::NewFromUtf8(isolate, str));
  tibems_int prio;
  tibemsMsg_GetPriority(response_msg, &prio);
  header->Set(String::NewFromUtf8(isolate, "JMSPriority"), Number::New(isolate, prio));
  tibemsDeliveryMode mode ;
  tibemsMsg_GetDeliveryMode(response_msg,&mode);
  if(mode == TIBEMS_NON_PERSISTENT){
    header->Set(String::NewFromUtf8(isolate, "JMSDeliveryMode"), String::NewFromUtf8(isolate, "TIBEMS_NON_PERSISTENT"));  
  }
  if(mode == TIBEMS_PERSISTENT){
    header->Set(String::NewFromUtf8(isolate, "JMSDeliveryMode"), String::NewFromUtf8(isolate, "TIBEMS_PERSISTENT"));  
  }
  if(mode == TIBEMS_RELIABLE){
    header->Set(String::NewFromUtf8(isolate, "JMSDeliveryMode"), String::NewFromUtf8(isolate, "TIBEMS_RELIABLE"));  
  }
  tibems_long timestamp;
  tibemsMsg_GetTimestamp(response_msg,&timestamp);
  header->Set(String::NewFromUtf8(isolate, "JMSTimestamp"), Number::New(isolate, timestamp));
  // tibemsMsgEnum* enumeration = new tibemsMsgEnum();
  // const char* prop_name;
  // tibemsMsgField field;
  // tibemsMsg_GetPropertyNames(msg,enumeration);
  // while((status = tibemsMsgEnum_GetNextName(*enumeration,&prop_name)) == TIBEMS_OK){
  //   tibemsMsg_GetProperty(msg,prop_name,&field);
  //   header->Set(String::NewFromUtf8(isolate, prop_name), String::NewFromUtf8(isolate, "a"));
  // }
  // tibemsMsgEnum_Destroy(*enumeration);
  // tibemsMsg_GetProperty(msg,"JMSExpiration",&field);
  // header->Set(String::NewFromUtf8(isolate, "JMSExpiration"), String::NewFromUtf8(isolate, (&field)->data.utf8Value));

  tibemsTextMsg_GetText(response_msg, &str);
  obj->Set(String::NewFromUtf8(isolate, "body"), String::NewFromUtf8(isolate, str));
  obj->Set(String::NewFromUtf8(isolate, "header"), header);


  /* destroy the response message */
  status = tibemsMsg_Destroy(response_msg);
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
  args.GetReturnValue().Set(obj);
}

void EmsConnection::sendToQueueSync(const FunctionCallbackInfo<Value>& args){
  sendToDestinationSync(false,args);
}
void EmsConnection::sendToTopicSync(const FunctionCallbackInfo<Value>& args){
  sendToDestinationSync(true,args);
}
void EmsConnection::requestFromQueueSync(const FunctionCallbackInfo<Value>& args){
  requestFromDestinationSync(false,args);
}
void EmsConnection::requestFromTopicSync(const FunctionCallbackInfo<Value>& args){
  requestFromDestinationSync(true,args);
}
}  // namespace nodeems
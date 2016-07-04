# node-ems
This plugin provides basic connectivity between node.js and Tibco EMS. So far this plugin is purely experimental and full of bugs. So don't use it for anything important.

You will still need the Tibco EMS Client installation. This plugin just wraps the c-based EMS client and exposes its functionality as node.js module.

### What does work
* send a text message to queue (request/reply and push)
* send a text message to topic (request/reply and push)

### What does not work
* using connection pools (currently every call establishes a new connection)
* transport encryption (no SSL at all)
* subscriptions (neither Queue nor Topic)
* QueueBrowser

### How does is work
```javascript
"use strict";
const ems = require('./build/Release/node-ems');

let server= "tcp://localhost:7222";
let user="admin";
let password="admin";
let queueName="queue.push";
let topicName="topic.push";
let header={};
let body="hello world";

var ems_conn = ems.prepare(server,user,password);
console.log(JSON.stringify(ems_conn,null,2));

//send a message to a queue -> fire and forget
var msgId1 = ems_conn.sendToQueueSync(queueName,header,body);
console.log("msgId1: "+msgId1);

//send message to a topic -> fire and forget
var msgId2 = ems_conn.sendToTopicSync(topicName,header,body);
console.log("msgId2: "+msgId2);

//send message to a queue and wait for a response -> request/reply
var response1 = ems_conn.requestFromQueueSync("queue.rr",header,body);
console.log("queue response: "+JSON.stringify(response1,null,2));

//send message to a topic and wait for a response -> request/reply
var response2 = ems_conn.requestFromTopicSync("topic.rr",header,body);
console.log("topic response: "+JSON.stringify(response2,null,2));

console.log('done');
```

### How to build it
1. setting the environment variables for EMS
  1. export EMS_HOME=/Users/jens/tibco/ems/8.0
  2. export DYLD_LIBRARY_PATH=DYLD_LIBRARY_PATH:/usr/lib:$EMS_HOME/lib
2. npm i JensWalter/node-ems

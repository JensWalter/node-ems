# node-ems
This plugin provides basic connectivity between node.js and Tibco EMS. So far this plugin is purely experimental and full of bugs. So don't use it for anything important.

### What does work
* send a text message to queue
* send a tex message to topic

### What does not work
* using connection pooling (currently every call establishes a new connection)
* transport encryption (no SSL at all)
* subscriptions (neither Queue nor Topic)
* QueueBrowser
* setting header properties of a message
* lots of mem leaks

### How does is work
```javascript
"use strict";
const ems = require('./build/Release/node-ems');

let server= "tcp://localhost:7222";
let user="admin";
let password="admin";
let queueName="queue.test";
let topicName="topic.test";
let header={};
let body="hello world";

let msgId = ems.sendMsgToQueueSync(server,user,password,queueName,header,body);
console.log(msgId);

msgId = ems.sendMsgToTopicSync(server,user,password,topicName,header,body);
console.log(msgId);
```

### How to build it
1. Check out the repository
2. modify the build.sh to point to your local EMS directory
3. run the build.sh

"use strict";
const ems = require('./build/Release/node-ems');

let server= "tcp://localhost:7222";
let user="admin";
let password="admin";
let queueName="queue.test";
let topicName="topic.test";
let header={};
let body="hello world";
var msgId = ems.sendMsgToQueueSync(server,user,password,queueName,header,body);
console.log(msgId);
var msgId = ems.sendMsgToTopicSync(server,user,password,topicName,header,body);
console.log(msgId);

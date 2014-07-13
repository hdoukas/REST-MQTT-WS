var http = require('http');
var fs = require('fs');


//create a WS server
var WebSocketServer = require('ws').Server
  , wss = new WebSocketServer({port: 8010});

  var WebSocket = require('ws');


//start a mqtt client
  var mqtt = require('mqtt')
  client = mqtt.createClient(1883, 'localhost');
  client.subscribe('message_in');



//when mqtt message is received
client.on('message', function (topic, message) {
      console.log(message);
      //dispatch it to the WS clients connected
        for(var i in wss.clients)
          wss.clients[i].send(message);
      
    });

//when WS client has connected:
wss.on('connection', function(ws) {
    //this.WebSocket = ws;  
    ws.on('message', function(message) {
        console.log('received: %s', message);
        client.publish('message', message);


    });
    
});


// This is a mock server that simulates just the position given by the socket. 
// TODO mock a third value in order to get the rotation angle for the arrow.
var express = require('express');
var server = express();

server.get('/', function(req, res) {
    res.send({
        x: 350,
        y: 350
    });
})
server.listen('3001', function() {
    console.log('Express is listening on port 3000');
});

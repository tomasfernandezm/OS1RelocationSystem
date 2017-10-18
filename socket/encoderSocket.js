var net = require('net');
var fs = require('fs');

var client = new net.Socket();
var server = net.createServer(function(socket) {
    socket.write('Aca tomi la variable socket tiene la fafa que te acaban de mandar mepa\r\n');

	socket.on('data', function(data){
		console.log(data);
        console.log(new Buffer(data, 'base64').toString("ascii"));
	});
});

server.listen(7001, '127.0.0.1');

console.log("Escuchando");


var ip = '';
var port = 7000;

console.log('Encoding...');
var file="./photo2.jpg";
    // read binary data
var bitmap = fs.readFileSync(file);
    // convert binary data to base64 encoded string
var buffer = new Buffer(bitmap).toString('base64');
console.log('Encoded...');

client.connect(port, ip, function() {
	console.log('Connected');
	console.log('Writing...');
	client.write(buffer);
	console.log('Wrote !!');
	client.end();
});

client.on('data', function(data) {
	console.log('Received: ' + data);
	client.destroy(); // kill client after server's response
});

client.on('close', function() {
	console.log('Connection closed');
});


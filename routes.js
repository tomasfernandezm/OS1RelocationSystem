var express = require('express');
var router = express.Router();
var multer = require('multer');
var net = require('net');

var storage = multer.memoryStorage();

var upload = multer({ storage : storage});

var coordinates;

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile(__dirname + "/index.html");
});

/* POST photo */
router.post('/photo',upload.single('imageUpload'), function(req, res){
    var uploadedFile = req.file.buffer;
    console.log(uploadedFile);

    var client = new net.Socket();
    var server = net.createServer(function(socket) {
        socket.write('Aca tomi la variable socket tiene la fafa que te acaban de mandar mepa\r\n');

        socket.on('data', function(data){
            console.log(data);
            var coor = new Buffer(data, 'base64').toString("ascii");
            console.log(coor);
            coordinates = coor.split(" ");
			res.render('map', {x: coordinates[0], y: coordinates[2]})
        });
    });

    server.listen(7001, '127.0.0.1');

    console.log("Escuchando");


    var ip = '';
    var port = 7000;

    console.log('Encoding...');
    var file="./photo2.jpg";
    // read binary data
    // var bitmap = fs.readFileSync(file);
    // convert binary data to base64 encoded string
    var buffer = new Buffer(uploadedFile).toString('base64');
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
});

module.exports = router;

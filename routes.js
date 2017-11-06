var express = require('express');
var router = express.Router();
var multer = require('multer');
var net = require('net');

var storage = multer.memoryStorage();

var upload = multer({ storage : storage});
var client = new net.Socket();
const ORIGIN_X =  240;
const ORIGIN_Y = 370;

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile(__dirname + "/index.html");
});

/* POST photo */
router.post('/photo',upload.single('imageUpload'), function(req, res){
    
    var uploadedFile = req.file.buffer;
    console.log(uploadedFile);

    var server = net.createServer(function(socket) {
        socket.on('data', function(data){
            console.log(data);
            var coor = new Buffer(data, 'base64').toString("ascii");
            console.log(coor);
            coordinates = coor.split(" ");
            const x = Math.floor(coordinates[0]*15 + ORIGIN_X);
            const y = Math.floor(coordinates[1]*15 + ORIGIN_Y);
			      res.render('map', {x: x, y: y});
            server.close();
        });
    });

    server.listen(7001, '127.0.0.1');

    var ip = '';
    var port = 7000;

    var buffer = new Buffer(uploadedFile).toString('base64');
    client.connect(port, ip, function() {
        console.log('Connected');
        console.log('Writing...');
		console.log(buffer.byteLength);
        client.write(buffer, function (err){
			console.log(err);
			console.log('Wrote !!');
        	client.end();
		});
    });
});

module.exports = router;

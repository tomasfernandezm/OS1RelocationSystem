var express = require('express');
var router = express.Router();
var multer = require('multer');

var storage =   multer.diskStorage({
  destination: function (req, file, callback) {
    callback(null, './uploads');
  },
  filename: function (req, file, callback) {
    callback(null, file.originalname);
  }
});

var upload = multer({ storage : storage}).single('imageUpload');

/* GET home page. */
router.get('/', function(req, res, next) {
  res.sendFile(__dirname + "/index.html");
});

/* POST photo */
router.post('/photo',function(req,res){
  upload(req,res,function(err) {
      if(err) {
          return res.end("Error uploading file.");
      }
      res.end("File is uploaded");
  });
});

module.exports = router;

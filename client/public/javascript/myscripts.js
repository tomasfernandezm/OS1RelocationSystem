function loader(){
    document.getElementById('imgPreview').setAttribute('hidden', 'hidden');
    document.getElementById('loader').style.visibility = 'visible';
}



function readURL(input) {

    if (input.files && input.files[0]) {
        var reader = new FileReader();

        reader.onload = function (e) {
            $('#imgPreview').attr('src', e.target.result);
        }
        reader.readAsDataURL(input.files[0]);
    }
}

$("#pic").change(function(){
    readURL(this);
});

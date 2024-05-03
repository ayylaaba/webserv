<?php
// Set the content type header to indicate that the response is an image
header('Content-Type: image/jpeg');

// // Path to the image file
$imagePath = '/nfs/homes/ayylaaba/Desktop/last_update6/website/img/admin.jpeg';

// // Output the image file
readfile($imagePath);
echo "hello"
?>
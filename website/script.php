<?php
// Set the content type header to indicate that the response is an image
header('Content-Type: image/jpeg');

// // Path to the image file
$imagePath = '/home/mallaoui/Desktop/web/website/folder/outfile_1714336839-975987.png';

// // Output the image file
readfile($imagePath);
echo "hello"
?>
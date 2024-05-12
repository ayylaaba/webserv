<?php
header('Content-Type: image/png');
$imagePath = '/home/mallaoui/Desktop/webserv/my_errors/files/418-im-a-teapot.png';
readfile($imagePath);
?>
<?php
header('Content-Type: image/avif');
$imagePath = '/nfs/homes/ayylaaba/Desktop/webserv/my_errors/img_404.avif';
readfile($imagePath);
?>
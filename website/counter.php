<?php

setcookie("TestCookie", "Value", time() + 3600, "/", "", false, true);

header("Content-Type: text/html");
echo "Cookie has been set.";

ob_start();
session_start();


echo "Current Session ID: " . session_id() . "<br>"; 
echo "Current Session 'id' Value Before: " . ($_SESSION["id"] ?? 'not set') . "<br>";

if (!isset($_SESSION["id"])) {
    $_SESSION["id"] = 0;
}

if (isset($_GET["add"])) {
    $_SESSION["id"]++;
}

echo "Current Session 'id' Value After: " . $_SESSION["id"] . "<br>";

?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="Cache-Control" content="no-cache, no-store, must-revalidate">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="0">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Your Page Title</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background-color: #f0f0f0;
            margin: 0;
            padding: 0;
        }

        h3 {
            text-align: center;
            color: #333;
        }

        form {
            text-align: center;
            margin-top: 20px;
        }

        input[type="submit"] {
            background-color: #007BFF;
            color: #fff;
            border: none;
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
            border-radius: 5px;
        }

        input[type="submit"]:hover {
            background-color: #0056b3;
        }
    </style>
</head>
<body>
    <h3><?php echo $_SESSION["id"]; ?></h3>
    <form action="" method="GET">
        <input type="submit" name="add" value="Add Number">
    </form>
</body>
</html>
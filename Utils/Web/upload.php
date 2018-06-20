<html>
<head>
	<title>Результат загрузки файла</title>
    <meta http-equiv="content-type" content="text/html; charset=UTF-8" />
</head>
<body>
<?php
   /*if($_FILES["file"]["size"] > 1024*3*1024)
   {
     echo ("Размер файла превышает три мегабайта");
     exit;
   }*/
   // Проверяем загружен ли файл
   if(is_uploaded_file($_FILES["file"]["tmp_name"]))
   {
	   usleep(100000);
     // Если файл загружен успешно, перемещаем его
     // из временной директории в конечную
	 $web_root = 'http://localhost/ImageUploader/Web';
	 
	 $dstFolder= "/files/";
	 $dst = $_FILES["file"]["name"];
	 $fileNameNoExt =  pathinfo ( $dst, PATHINFO_FILENAME);
     move_uploaded_file($_FILES["file"]["tmp_name"], dirname(__FILE__).$dstFolder.$dst);
	 $direct_link = $web_root . $dstFolder. $dst;
	 $thumb_link = $web_root. $dstFolder. $fileNameNoExt .'_thumb.png';
	 echo "[url=$direct_link][img]".$thumb_link.'[/img][/url]';
   } else {
      echo("Ошибка загрузки файла");
   }
?>
</body>
</html>
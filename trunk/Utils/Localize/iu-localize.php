<head>
<title> Image Uploader Localization Script</title></head>
<body>
<h1>Image Uploader Localization Script</h1>
<?
  $dir = $_POST["dir"];
	$lang_dir = $_POST["lang_dir"];
  if(!$dir) 
	{
		die ('<form action=iu-localize.php method=post>
	Image Uploader sources directory (e.g. <b><a href=# onclick="document.forms[0].dir.value=\'D:\\\\Projects\\\\Image Uploader\\\\Source\'"> D:\Projects\Image Uploader\Source\</a></b>):<br> 
	<input type=edit size=50 name=dir><p>
	
	Image Uploader languages directory (e.g. <b><a href=# onclick="document.forms[0].lang_dir.value=\'D:\\\Projects\\\Image Uploader\\\Build\\\release\\\Lang\'"> D:\Projects\Image Uploader\Build\release\Lang\</a></b>):<br> 
	<input type=edit size=50 name=lang_dir><p>
	<input type=submit value=Localize name=btn_submit>
	</form>');
	}
?>

<p> <h3>Doing localization...</h3>


<?

function write_header($f)
{
  fwrite($f,  "\xff\xfe");
  $head = "# This generated file is Image Uploader's language file. It must be saved in UTF-16LE encoding.\r\n";
  $head_utf16 = mb_convert_encoding($head, "UTF-16LE","Windows-1251");	
  fwrite($f, $head_utf16);
}

$f = fopen($lang_dir."\\default.lng","w");

if(!$f) die("Could not create output file");
write_header($f);

$hashes = array();
$strings = array();

function int2hex($intega)
{
    $Ziffer = "0123456789ABCDEF";
	return $Ziffer[($intega%256)/16].$Ziffer[$intega%16];
}

function myhash($key)
{
	$hash = 222;
	$len = strlen($key);

	for ($i=0; $i<$len; ++$i) 
	{
      $hash = ($hash ^ ord($key[$i])) + (($hash<<26)+($hash>>6));
	}
	return $hash;
}

function add_string($str)
{
	$code = 'return "'.$str.'";';
  //echo htmlspecialchars($code)."<br>";
  $str = eval($code);

	global $hashes, $strings;
	foreach($strings as $item) { if($item==$str) {return;}}

	$item_u = mb_convert_encoding($str, "UTF-16LE","Windows-1251");
	$hashes[]=dump_dword(myhash($item_u));
	$str =  str_ireplace("\n", "\\n", $str);
	$strings[] = $str;
}

function dump_dword($hash)
{
	$f=fopen("aaaa","a+");
	$hash= pack("l",$hash);
	fwrite($f, $hash);
	fclose($f);
	$res= sprintf("%02x%02x%02x%02x",ord($hash[0]),ord($hash[1]),ord($hash[2]),ord($hash[3]));
	return $res;
}

function parse_language_file($filename,$hashes,$stringAr)
{
	$file =fopen($filename.".new","w");
	write_header($file);
	
	$data = file_get_contents($filename);
	$strings= explode("\r\0\n\0", $data);


	foreach($strings as $key => $item)
	{
		if($key==0) {
		$item = ltrim($item , "\xff\xfe"); }
		
		$dd = explode("=\0", $item);
		$hash = $dd[0];	
		
		$hash = trim(mb_convert_encoding($hash,"Windows-1251","UTF-16LE"));
		
		
		 $k = array_search($hash, $hashes);  
		if(!($k=== false))
		{
			fwrite($file, $item."\r\0\n\0");
			unset($hashes[$k]);
			unset($stringAr[$k]);
		}
	}

	foreach($stringAr as $ki => $it)
	{
		$rrr=$hashes[$ki]." = $it\r\n";
		$str = mb_convert_encoding($rrr, "UTF-16LE","Windows-1251");
		
		fwrite($file, $str);		
		echo "<br> Not found>> $it";
	}
	
	fclose($file);
}

function parse_source_file($filename,$ff)
{
  $content = file_get_contents($filename);//"/(?<=TR\(\")[\S]*(=\"\))/i"
  preg_match_all("/TR\(\"(.*?[^\\x5c])\"\)/", $content, $matches);

	foreach($matches[1] as $item) {
	    //echo "<br>$item";
	add_string($item);
	}
	
	
	 preg_match_all("/TRC\((.*?),[ ]*\"(.*?[^\\x5c])\"\)/", $content, $matches);
	
	foreach($matches[2] as $item) {
	    //echo "<br><font color=green>$item</font>";
	add_string($item);
	}

}


$path = $dir ;
//$path = $path . "$sub";
$dh = opendir($path);
$i=2;

while (($file = readdir($dh)) !== false) {
    if($file != "." && $file != "..") {

            if (substr($file, -4, 4) ==".cpp" or substr($file, -2, 2) ==".h"){
            echo "<p><B>$i. Parsing $file<br /></b>";
		parse_source_file($path.'\\'.$file,$f);
            }else{           
        ;
          
}
        $i++;
    }
}

print "<p><b>Total: $i source files parsed</b>";

closedir($dh);

$count = 0;echo "<p>Saving to file...</p>";
foreach($strings as $item) { 

$item_u = mb_convert_encoding($item, "UTF-16LE","Windows-1251");

$hass= myhash($item_u);

$rrr= dump_dword($hass)." = $item\r\n";
//echo "$item, $hass, $rrr<br>";
$str = mb_convert_encoding($rrr, "UTF-16LE","Windows-1251");

fwrite($f,  $str);
$count++;
}
echo "Total $count language records";

fclose($f);


if($lang_dir) 
{
echo "<p><h3>Parsing other language files (in directory \"".stripslashes($lang_dir)."\") </h3>";


$dh = opendir($lang_dir);
$i=1;

while (($file = readdir($dh)) !== false) {
    if($file != "." && $file != "..") {

            if (substr($file, -4, 4) ==".lng" && $file!="default.lng"){
            echo "<p><B>$i. Parsing $file<br /></b>";
		parse_language_file($lang_dir.'\\'.$file,$hashes,$strings);
$i++;
            }else{           
        ;
          
}
        
    }
}
$i--;
print "<p><b>Total: $i language files parsed</b>";

closedir($dh);


}


?>
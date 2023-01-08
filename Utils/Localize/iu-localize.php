<?php 

error_reporting(E_ALL & ~E_NOTICE);
if (PHP_INT_SIZE !== 8) {
    die('This script should be executed by 64-bit PHP engine');
}
?>
<head>
    <title> Image Uploader Localization Script</title></head>
<body>
<h1>Image Uploader Localization Script</h1>
<?php
    $dir = isset( $_POST["dir"] ) ? $_POST["dir"] : null;
    $lang_dir = isset( $_POST["lang_dir"] ) ? $_POST["lang_dir"] : null;
    if ( !$dir ) {
        die ( '<form action=iu-localize.php method=post>
	Image Uploader sources directory (e.g. <b><a href=# onclick="document.forms[0].dir.value=this.innerHTML">d:\Develop\imageuploader\Source\</a></b>):<br>
	<input type="text" size=50 name=dir value="/mnt/d/Develop/image-uploader/Source/"><p>
	
	Image Uploader languages directory (e.g. <b><a href=# onclick="document.forms[0].lang_dir.value=this.innerHTML">d:\Develop\imageuploader\Lang\</a></b>):<br> 
	<input type="text" size=50 name=lang_dir value="/mnt/d/Develop/image-uploader/Lang/"><p>
	<input type=submit value=Localize name=btn_submit>
	</form>' );
    }
?>

<p>

<h3>Doing localization...</h3>


<?php
	function write_header( $f ) {
        $head = "# This generated file is Image Uploader's language file. It must be saved in UTF-8 encoding. 777011a3 = translator's name (and optional e-mail, website)\r\n";
        fwrite( $f, $head );
    }

	$f = fopen( $lang_dir . "/English.lng.src", "w" );

	if ( !$f ) {
        die( "Could not create output file" );
    }
	write_header( $f );

	$hashes = array( );

	/*function int2hex( $intega ) {
        $Ziffer = "0123456789ABCDEF";
        return $Ziffer[( $intega % 256 ) / 16] . $Ziffer[$intega % 16];
    }*/

    function overflow32($in) {
        return unpack('l', pack('i', $in & 0xFFFFFFFF))[1];
    }


    function myhash( $key ) {
        $hash = 222;
        $len = strlen( $key );

        for ( $i = 0; $i < $len; ++$i )
        {
            $hash = overflow32($hash) ;
            $hash = ( $hash ^ ord( $key[$i] ) ) +overflow32 (( ( $hash << 26 ) + ( $hash >> 6 ) ));
        }
        $hash = overflow32($hash );
        return $hash;
    }

	function add_string( $str ) {
        $code = 'return "' . $str . '";';
        //echo htmlspecialchars($code)."<br>";
        $str = eval( $code );

        global $hashes;

        /*foreach ( $strings as $item ) {
            if ( $item == $str ) {
                return;
            }
        }*/

        $item_u = mb_convert_encoding( $str, "UTF-16LE", "UTF-8" );
        $str = str_ireplace( "\r\n", "\\n", $str );
        $str = str_ireplace( "\n", "\\n", $str );
        $key = dump_dword( myhash( $item_u ) );
        if (array_key_exists($key, $hashes) && $hashes[$key] !== $str) {
            // in case of collision
            echo "<br>warning: $key =".$str;
        }
        $hashes[$key] = $str;

        //$strings[] = ;
    }

	function dump_dword( $hash ) {
        $hash = pack( "l", $hash );

        $res = sprintf( "%02x%02x%02x%02x", ord( $hash[0] ), ord( $hash[1] ), ord( $hash[2] ), ord( $hash[3] ) );
        return $res;
    }

	function read_language_file( $filename ) {
        $result = array( );
        $data = trim( file_get_contents( $filename ) );
        //$data = trim( mb_convert_encoding( $data, "UTF-8", "UTF-16LE" ) );
        $strings = explode( "\r\n", $data );
        //var_dump($strings);
        foreach ( $strings as $key => $item )
        {
            /*if ( $key == 0 ) {
                $item = ltrim( $item, "\xff\xfe" ); remove BOM
            }*/
            $dd = explode( "= ", $item );
            $hash = trim( $dd[0] );
            $value = $dd[1];
            $result[$hash] = $value;
        }
        return $result;
    }
	
	function parse_language_file( $path, $filename, $hashes ) {
        $file = fopen( $filename . ".new", "w" );
        write_header( $file );
        $data = file_get_contents( $filename );
        $strings = explode( "\r\n", $data );

        $csv = fopen($filename . ".csv", "w" );

        if ( $filename != "default" && $filename != "English" ) {
            $english_strings = read_language_file( $path . "/English.lng.src" );
            //var_dump( $english_strings );
        }

        foreach ( $strings as $key => $item )
        {
            $dd = explode( "=", $item );
            $hash = $dd[0];
            $val = trim($dd[1]);
            $hash = trim( $hash );
            $english = isset($english_strings[$hash])?$english_strings[$hash]:null;
            $k = isset($hashes[$hash]) ? $hashes[$hash] : false;
            if ( $hash === 'language' || $hash === 'RTL' || !( $k === false ) ) {
                fwrite( $file, $item . "\r\n" );
                if ($english !== $val && trim($english) !== $val && $english !== null) {
                    $english = str_ireplace("\\r\\n", "\r\n", $english);
                    $english = str_ireplace("\\n", "\n", $english);

                    $val = str_ireplace("\\r\\n", "\n", $val);
                    $val = str_ireplace("\\n", "\n", $val);
                    fputcsv($csv, ['',$english, $val]);
                }
                unset( $hashes[$hash] );
            }
        }

        foreach ( $hashes as $ki => $it )
        {
            $value = isset($english_strings[$ki])?$english_strings[$ki]:"";
            if ( $value == "" ) {
                $value = $it;
            }
            $str = $ki . " = $value\r\n";

            fwrite( $file, $str );
            echo "<br> Not found>> $it ($value)";
        }

        fclose( $file );
        fclose($csv);
    }

	function parse_source_file( $filename ) {
        $content = file_get_contents( $filename ); //"/(?<=TR\(\")[\S]*(=\"\))/i"
        preg_match_all( "/TR\(\"(.*?[^\\x5c])\"\)/i", $content, $matches );

        foreach ( $matches[1] as $item ) {
            //echo "<br>$item";
            add_string( $item );
        }
        
        
        preg_match_all( "/TR_CONST\(\"(.*?[^\\x5c])\"\)/", $content, $matches );

        foreach ( $matches[1] as $item ) {
            //echo "<br>$item";
            add_string( $item );
        }

        preg_match_all( "/TRC\((.*?),[ ]*\"(.*?[^\\x5c])\"\)/", $content, $matches );

        foreach ( $matches[2] as $item ) {
            //echo "<br><font color=green>$item</font>";
            add_string( $item );
        }
    }

	function parse_dir( $path ) {
        $directory = new RecursiveDirectoryIterator($path);
        $iterator = new RecursiveIteratorIterator($directory);

        foreach ($iterator as $item) {
            /** @var SplFileInfo $item */
            $file = $item->getPathname();
            $fileName = $item->getFilename();

            if ( $fileName != "." && $fileName != ".." ) {
                //echo "<br>".$fileName ."<br>";
                if ( substr( $fileName, -4, 4 ) == ".cpp" || substr( $fileName, -2, 2 ) == ".h" ) {
                    echo "<p><B>$i. Parsing $file<br /></b>";
                    parse_source_file( $file );
                }
                /*else {
                    if ( is_dir( $path . '\\' . $file ) && $file != 'qimageuploader' ) {

                        echo "<p><B>$i. Parsing subdir $file<br /></b>";
                        parse_dir( $path . '\\' . $file . '\\' );

                    }
                }*/
                $i++;
            }
        }
        /*$dh = opendir( $path );
        $i = 2;
        while ( ( $file = readdir( $dh ) ) !== false ) {
            if ( $file != "." && $file != ".." ) {
                if ( substr( $file, -4, 4 ) == ".cpp" or substr( $file, -2, 2 ) == ".h" ) {
                    echo "<p><B>$i. Parsing $file<br /></b>";
                    parse_source_file( $path . '\\' . $file );
                }
                else {
                    if ( is_dir( $path . '\\' . $file ) && $file != 'qimageuploader' ) {
                        
                        echo "<p><B>$i. Parsing subdir $file<br /></b>";
                        parse_dir( $path . '\\' . $file . '\\' );

                    }
                }
                $i++;
            }
        }
        closedir( $dh );*/
    }

 parse_dir( $dir );
print "<p><b>All source files parsed</b>";

$count = 0;
echo "<p>Saving to file...</p>";
foreach ( $hashes as $key => $item ) {

    //$item_u = mb_convert_encoding( $item, "UTF-16LE", "UTF-8" );
    //var_dump($item_u);
    //echo "<br>";
    //$hass = myhash( $item_u );

    $str = $key . " = $item\r\n";
    //echo "$item, $hass, $rrr<br>";

    fwrite( $f, $str );
    $count++;
}
echo "Total $count language records";

fclose( $f );

    var_dump($hashes);
	if ( $lang_dir ) {
        echo "<p><h3>Parsing other language files (in directory \"" . stripslashes( $lang_dir ) . "\") </h3>";
        $dh = opendir( $lang_dir );
        $i = 1;

        while ( ( $file = readdir( $dh ) ) !== false )
        {
            if ( $file != "." && $file != ".." ) {
                if ( substr( $file, -4, 4 ) == ".lng" && $file != "English.lng.src" ) {
                    echo "<p><B>$i. Parsing $file<br /></b>";
                    parse_language_file( $lang_dir, $lang_dir . '/' . $file, $hashes );
                    $i++;
                }
                else {
                    ;

                }
            }
        }
        $i--;
        print "<p><b>Total: $i language files parsed</b>";
        closedir( $dh );
    }


?>
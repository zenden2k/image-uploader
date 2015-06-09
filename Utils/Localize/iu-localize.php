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
	<input type=edit size=50 name=dir value="d:\Develop\imageuploader\Source\"><p>
	
	Image Uploader languages directory (e.g. <b><a href=# onclick="document.forms[0].lang_dir.value=this.innerHTML">d:\Develop\imageuploader\Lang\</a></b>):<br> 
	<input type=edit size=50 name=lang_dir value="d:\Develop\imageuploader\Lang\"><p>
	<input type=submit value=Localize name=btn_submit>
	</form>' );
    }
?>

<p>

<h3>Doing localization...</h3>


<?php
	function write_header( $f ) {
        fwrite( $f, "\xff\xfe" );
        $head = "# This generated file is Image Uploader's language file. It must be saved in UTF-16LE encoding. 777011a3 = translator's name (and optional e-mail, website)\r\n";
        $head_utf16 = mb_convert_encoding( $head, "UTF-16LE", "Windows-1251" );
        fwrite( $f, $head_utf16 );
    }

	$f = fopen( $lang_dir . "\\English.lng.src", "w" );

	if ( !$f ) {
        die( "Could not create output file" );
    }
	write_header( $f );

	$hashes = array( );

	function int2hex( $intega ) {
        $Ziffer = "0123456789ABCDEF";
        return $Ziffer[( $intega % 256 ) / 16] . $Ziffer[$intega % 16];
    }

	function myhash( $key ) {
        $hash = 222;
        $len = strlen( $key );

        for ( $i = 0; $i < $len; ++$i )
        {
            $hash = ( $hash ^ ord( $key[$i] ) ) + ( ( $hash << 26 ) + ( $hash >> 6 ) );
        }
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

        $item_u = mb_convert_encoding( $str, "UTF-16LE", "Windows-1251" );
        $str = str_ireplace( "\n", "\\n", $str );
        $hashes[dump_dword( myhash( $item_u ) )] = $str;

        //$strings[] = ;
    }

	function dump_dword( $hash ) {
        //$f = fopen( "aaaa", "a+" );
        $hash = pack( "l", $hash );
        //fwrite( $f, $hash );
        //fclose( $f );
        $res = sprintf( "%02x%02x%02x%02x", ord( $hash[0] ), ord( $hash[1] ), ord( $hash[2] ), ord( $hash[3] ) );
        return $res;
    }

	function read_language_file( $filename ) {
        $result = array( );
        $data = file_get_contents( $filename );
        $data = trim( mb_convert_encoding( $data, "Windows-1251", "UTF-16LE" ) );
        $strings = explode( "\r\n", $data );

        foreach ( $strings as $key => $item )
        {
            if ( $key == 0 ) {
                $item = ltrim( $item, "\xff\xfe" );
            }
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
        $strings = explode( "\r\0\n\0", $data );


        if ( $filename != "default" && $filename != "English" ) {
            $english_strings = read_language_file( $path . "\\English.lng" );
            //echo "OLOLO<br>";
            //var_dump( $english_strings );
        }

        foreach ( $strings as $key => $item )
        {
            if ( $key == 0 ) {
                $item = ltrim( $item, "\xff\xfe" );
            }

            $dd = explode( "=", $item );
            $hash = $dd[0];

            $hash = trim( mb_convert_encoding( $hash, "Windows-1251", "UTF-16LE" ) );

            $k = isset($hashes[$hash]) ? $hashes[$hash] : false;
            if ( $hash === 'language' || !( $k === false ) ) {
                fwrite( $file, $item . "\r\0\n\0" );
                unset( $hashes[$hash] );
            }
        }

        foreach ( $hashes as $ki => $it )
        {
            $value = $english_strings[$ki];
            if ( $value == "" ) {
                $value = $it;
            }
            $rrr = $ki . " = $value\r\n";
            $str = mb_convert_encoding( $rrr, "UTF-16LE", "Windows-1251" );

            fwrite( $file, $str );
            echo "<br> Not found>> $it ($value)";
        }

        fclose( $file );
    }

	function parse_source_file( $filename ) {
        $content = file_get_contents( $filename ); //"/(?<=TR\(\")[\S]*(=\"\))/i"
        preg_match_all( "/TR\(\"(.*?[^\\x5c])\"\)/", $content, $matches );

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
        $dh = opendir( $path );
        $i = 2;
        while ( ( $file = readdir( $dh ) ) !== false ) {
            if ( $file != "." && $file != ".." ) {
                if ( substr( $file, -4, 4 ) == ".cpp" or substr( $file, -2, 2 ) == ".h" ) {
                    echo "<p><B>$i. Parsing $file<br /></b>";
                    parse_source_file( $path . '\\' . $file );
                }
                else {
                    if ( is_dir( $path . '\\' . $file ) ) {
                        echo "<p><B>$i. Parsing subdir $file<br /></b>";
                        parse_dir( $path . '\\' . $file . '\\' );

                    }
                }
                $i++;
            }
        }
        closedir( $dh );
    }

 parse_dir( $dir );
print "<p><b>Total: $i source files parsed</b>";



$count = 0;echo "<p>Saving to file...</p>";
foreach ( $hashes as $key => $item ) {

    $item_u = mb_convert_encoding( $item, "UTF-16LE", "Windows-1251" );

    $hass = myhash( $item_u );

    $rrr = dump_dword( $hass ) . " = $item\r\n";
    //echo "$item, $hass, $rrr<br>";
    $str = mb_convert_encoding( $rrr, "UTF-16LE", "Windows-1251" );

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
                    parse_language_file( $lang_dir, $lang_dir . '\\' . $file, $hashes );
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
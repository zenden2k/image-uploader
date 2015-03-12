<?php
const SRCDIR = '../portable/temp/';

function clearDirectory($dir) {
    if (!file_exists($dir)) {
        return true;
    }

    if (!is_dir($dir)) {

        return unlink($dir);
    }

    foreach (scandir($dir) as $item) {
        if ($item == '.' || $item == '..') {
            continue;
        }

        if (!clearDirectory($dir . DIRECTORY_SEPARATOR . $item)) {
            return false;
        }

    }

    //return rmdir($dir);
}

class UpdateGenerator {
    private $sourcePackageFileName;
    private $srcDir;

    function __construct($packageName,$srcDir)
    {
        $sourcePackageFileName = 'iu_'.$packageName.'.xml';
        if ( !file_exists($sourcePackageFileName) ) {
            throw new Exception("Source Package $sourcePackageFileName not found");
        }
        $this->srcDir = $srcDir;
        $this->sourcePackageFileName = $sourcePackageFileName;
    }

    public function generate() {
        $root = simplexml_load_file($this->sourcePackageFileName);
        if ( $root->getName() !== 'UpdatePackageSource' ) {
            throw new Exception("Xml Root should be UpdatePackageSource");
        }

        /** @var SimpleXMLElement $entries */
        $entries = $root->Entries;
        if ( !$entries ) {
            throw new Exception("Entries node not found");
        }
        $packageName = $root['Name'];
        clearDirectory($packageName);
        if (!is_dir($packageName)) {
            mkdir($packageName);
        }
        $timestamp = time();
        $zip = new ZipArchive();
        $downloadPath = $root['DownloadPath'];

        $package_name_with_timestamp = "{$packageName}_$timestamp.".(strpos($downloadPath, 'zenden.ws')!== false ? "dat" : 'zip');
        $zipFileName = "$packageName/$package_name_with_timestamp";

        if ($zip->open($zipFileName, ZipArchive::CREATE)!==TRUE) {
            throw new Exception("Unable to create zip file <$zipFileName>\n");
        }

        $outPackageXml = new SimpleXMLElement('<UpdatePackage></UpdatePackage>');
        $downloadUrl = $downloadPath . $package_name_with_timestamp;
        $outPackageXml->addAttribute("Name", $packageName);
        $outPackageXml->addAttribute("UpdateUrl", $root['UpdateUrl']);
        $outPackageXml->addAttribute("CoreUpdate", $root['CoreUpdate']);
        $outPackageXml->addAttribute("DisplayName", $root['DisplayName']);
        $outPackageXml->addAttribute("DownloadUrl", $downloadUrl);

        $outEntries = $outPackageXml->addChild('Entries');
        foreach ( $entries->Entry as $entry ) {
            /** @var SimpleXMLElement $entry */
            $name = trim($entry['Name']);
            $saveToFolder = trim($entry['SaveToFolder']);
            $action = trim($entry['Action']);
            if (!$action ) {
                $action = 'add';
            }
            $foundFiles = glob($this->srcDir.$name);
           // echo 'Glob '.$this->srcDir.$name.'<br>';
            foreach ( $foundFiles as $srcFile ) {
                if ( !is_file($srcFile)) {
                    continue;
                }
               // echo $srcFile.'<br>';
                $md5 = md5_file($srcFile);
                $file = str_replace($this->srcDir,'',$srcFile);
                $newEntry = $outEntries->addChild('Entry');
                $newEntry->addAttribute('Name', $file);
                $newEntry->addAttribute('SaveTo', $saveToFolder. pathinfo($file, PATHINFO_BASENAME));
                $newEntry->addAttribute('Action', $action);
                $newEntry->addAttribute('Hash', $md5);
              //  echo $file.'<br>';
                $zip->addFile($srcFile,$file);
            }
            //var_dump($foundFiles);
           // echo $name.'<br>';
        }
        $outPackageXml->addChild('Info',$root->Info);
        $dom = new DOMDocument("1.0",'UTF-8');

        $dom->loadXML($outPackageXml->asXML());
        $dom->preserveWhiteSpace = false;
        $dom->standalone = true;
        $dom->formatOutput = true;
        $dom->encoding = 'UTF-8';

        //echo '<pre>'.htmlspecialchars( $dom->saveXML()).'</pre>';

        $zip->addFromString("package.xml",  $dom->saveXML());


        echo "Zip file '$zipFileName': numfiles: " . $zip->numFiles . ";\n";
        echo "status:" . $zip->status . "\n;";
        $zip->close();
        $outUpdateInfoXml = new SimpleXMLElement('<UpdateInfo></UpdateInfo>');
        $outUpdateInfoXml->addAttribute('Name', $packageName);
        $outUpdateInfoXml->addAttribute('DownloadUrl',$downloadUrl);
        $outUpdateInfoXml->addAttribute('TimeStamp',$timestamp);
        $outUpdateInfoXml->addAttribute("UpdateUrl", $root['UpdateUrl']);
        $outUpdateInfoXml->addAttribute("CoreUpdate", $root['CoreUpdate']);
        $outUpdateInfoXml->addAttribute("DisplayName", $root['DisplayName']);
        $outUpdateInfoXml->addAttribute("Hash", md5_file($zipFileName));
        $outUpdateInfoXml->addChild('Info',$root->Info);


        $dom = new DOMDocument("1.0",'UTF-8');

        $dom->loadXML($outUpdateInfoXml->asXML());
        $dom->preserveWhiteSpace = false;
        $dom->standalone = true;
        $dom->formatOutput = true;
        $dom->encoding = 'UTF-8';

        echo '<pre>'.htmlspecialchars( $dom->saveXML()).'</pre>';
        $dom->save("$packageName/$packageName.xml");
        $dom->save(SRCDIR."Data/Update/$packageName.xml");
        $dom->save("../../Data/Update/$packageName.xml");

    }
}

$package = isset($_GET['f']) ? $_GET['f'] : 'serversinfo';
$generator = new UpdateGenerator($package, SRCDIR);
$generator->generate();
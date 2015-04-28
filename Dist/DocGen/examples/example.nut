test <- "example"; // global variable

/**
* Function which acually doing upload of a file
* @var string pathToFile 
* @var CIUUploadParams options
* @return int - success(1), failure(0)
**/
function  UploadFile(pathToFile, options)
{
    nm.setUrl("http://example.com/upload.php");
    nm.addQueryParamFile("file", pathToFile, ExtractFileName(pathToFile),"");
    nm.addQueryParam("submit", "Upload file!");
    nm.doUploadMultipartData();

    local response = nm.responseBody(); // 'local' it's like javascript's 'var' but only for local variables
    local directUrl = regex_simple(response, "\\[IMG\\](.+)\\[/IMG\\]",0);
    
    options.setDirectUrl(directUrl);

    return 1; //успех
}

/** 
* Вспомогательная функция, упрощающая работу с регулярными выражениями
* @var string data - строка, в которой мы ищем
* @var string regStr - регулярное выражение, в формате, поддерживаемом стандартной библиотекой языка squirrel.
*      http://www.squirrel-lang.org/doc/sqstdlib2.html#d0e2580
*      Этот формат не поддерживает некоторые возможности формата PCRE, используемого в servers.xml.
* @var int start - с какой позиции в строке начинать поиск)
* @return string - возвращает текст, захваченный первой подмаской (subpattern).
**/
function regex_simple(data,regStr,start)
{
    local ex = regexp(regStr);
    local res = ex.capture(data, start);
    local resultStr = "";
    if(res != null){    
        resultStr = data.slice(res[1].begin, res[1].end);
    }
    return resultStr;
}

/** 
* Optional function:
* Retreving folder (album) list from server
* @var CFolderList list
* @return int - success(1), failure(0) 
**/
function GetFolderList(list)
{
    // Ваш код
    return 1; //успех
}
 
/** 
* Create an folder or an album
* @var CFolderItem parentAlbum
* @var CFolderItem album
* @return int - success(1), failure(0)
**/
function CreateFolder(parentAlbum,album)
{
    // Ваш код
    return 1; //успех
}
 
/** Modify a folder or an album (update name, description)
* @var CFolderItem album
* @return int - success(1), failure(0) 
**/
function ModifyFolder(album)
{
    // Ваш код
    return 1; //успех
}
 
// Функция, возвращающая список видов ограничений доступа к альбому:
// приватный, общедоступный, только для друзей и т.п.
// @return array 
function GetFolderAccessTypeList()
{
    return ["ТипДоступа1", "ТипДоступа2"];
}

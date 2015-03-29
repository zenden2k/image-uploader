

function tr(key, text) {
	try {
		return Translate(key, text);
	}
	catch(ex) {
		return text;
	}
}

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

function str_replace(str, pattern, replace_with)
{
	local resultStr = str;	
	local res;
	local start = 0;

	while( (res = resultStr.find(pattern,start)) != null ) {	

		resultStr = resultStr.slice(0,res) +replace_with+ resultStr.slice(res + pattern.len());
		start = res + replace_with.len();
	}
	return resultStr;
}

class Test {
	width = 10;
	constructor(params) {
		DebugMessage(width.tostring());
	}
	function callback(data) {
		DebugMessage(data.url);
		DebugMessage(width.tostring());
	}
	
}
/*function callback(data) {
	DebugMessage(data.url);
}*/
function OnUrlChangedCallback(data) {
	//DebugMessage(data.url);
}

function OnLoadFinished(data) {
	local br = data.browser;
	print(br.title());
}


function OnNavigateError(data) {
	DebugMessage(data.statusCode.tostring());
}
function  UploadFile(FileName, options)
{
	local browser = WebBrowser();
	browser.setTitle(tr("vk.browser.title", "Vk.com authorization"))
	browser.setOnUrlChangedCallback(OnUrlChangedCallback, null);
	browser.setOnNavigateErrorCallback(OnNavigateError, null);
	browser.setOnLoadFinishedCallback(OnLoadFinished, null);
	browser.navigateToUrl("http://ya.ru/fewfefew");
	browser.openModal();
	return 1;
}
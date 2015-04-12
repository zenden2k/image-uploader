fileName <- "";
siteRegexp <- CRegExp("http://imgbox.com/", "i");
//afterUploadRegexp <- CRegExp("index\\.sema?.*sa=kod", "");

opt <- null;
uploadResult <- 0;

function OnLoadFinished(data) {
	local doc = data.browser.document();
	local rootEl = doc.rootElement();
	/*if ( afterUploadRegexp.match(data.url) ) {
		return;
	} else*/ if ( siteRegexp.match(data.url ) ) {
		local text = rootEl.getInnerText();
		if ( text.find("Links Only",0) != null ) {
			local bbcode = doc.getElementById("code-bb-thumb").getInnerText();
			if ( !bbcode ) {
				data.browser.close();
			}
			local reg = CRegExp("\\[url=(.+?)\\]\\[img\\](.*?)\\[\\/img\\]", "i");
			if ( reg.match(bbcode) ) {
				opt.setThumbUrl(reg.getMatch(1));
				opt.setViewUrl(reg.getMatch(0));
				data.browser.runJavaScript("tc('f');");
				bbcode = doc.getElementById("code-bb-full").getInnerText();
				local reg2 = CRegExp("\\[url=(.+?)\\]\\[img\\](.*?)\\[\\/img\\]", "i");
				if ( reg2.match(bbcode) ) {
					opt.setDirectUrl(reg2.getMatch(1));
				}
				uploadResult = 1;
				data.browser.close();
			}
		} else {
			local form = doc.getElementById("upload-form");
			if ( !form.isNull() ) {
				local formElements = form.getFormElements();
				local fileInputSet = false;
				for ( local i = 0; i < formElements.len(); i++ ) {
					local name = formElements[i].getAttribute("name");
					local type = formElements[i].getAttribute("type");
					if ( type == "file" && !fileInputSet ) {
						formElements[i].setValue(fileName);
						fileInputSet = true;
					}
				}
			} else {
				//data.browser.close();
			}
		}	
	}

}



function OnFileInputFilledCallback(data) {
	local doc = data.browser.document();
	data.browser.runJavaScript( 
	    "$('#dropdown-content-type').val(1);" // family same content
        "$('#thumbnail-option').val('200c');"  // 200x200 
	    "$('#gallery-option').val('3');"  // do not create new gallery
	);
	local submitBtn = doc.getElementById("fake-submit-button");
	if ( submitBtn.isNull() ) {
		data.browser.close();
		return;
	}
	submitBtn.click();
}

function OnNavigateErrorCallback(data) {
	data.browser.close();
}

function  UploadFile(FileName, options)
{
	uploadResult = 0;
	opt = options;
	fileName = FileName;
	local webBrowser = CWebBrowser();
	if ( webBrowser.getMajorVersion() < 8 ) {
		WriteLog("error", "You need to install Internet Explorer 8 or later to upload to this server.");
		return -1;
	}
	webBrowser.setSilent(true);
	webBrowser.addTrustedSite("http://imgbox.com");
	webBrowser.navigateToUrl("http://imgbox.com");
	webBrowser.setOnLoadFinishedCallback(OnLoadFinished, null);
	webBrowser.setOnFileInputFilledCallback(OnFileInputFilledCallback, null);
	webBrowser.setOnNavigateErrorCallback(OnNavigateErrorCallback, null);

	webBrowser.exec();
	//webBrowser.showModal();
	return uploadResult;
}

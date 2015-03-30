#include "HtmlElementPrivate_win.h"
#include <Func/WinUtils.h>
namespace ScriptAPI {

void HtmlElementPrivate::setValue(const std::string& value) {
		CComQIPtr<IHTMLInputElement>  input = disp_ ? CComQIPtr<IHTMLInputElement> (disp_) : CComQIPtr<IHTMLInputElement> (elem_);
		if ( !input ) {
			LOG(WARNING) << "setValue: element is not an input.";
			return;
		}

		CComQIPtr<IHTMLInputFileElement>  inputFile = disp_ ? CComQIPtr<IHTMLInputFileElement> (disp_) : CComQIPtr<IHTMLInputFileElement> (elem_);
		if ( inputFile ) {
			CString val = IuCoreUtils::Utf8ToWstring(value).c_str();
			WinUtils::CopyTextToClipboard(val);
			VARIANT res;
			CComVariant comV(val);
			// Copy Full FileName To Clipboard
			//Clipboard()->SetTextBuf(sFile.c_str());
			inputFile->select();
			// Paste from ClipBoard to "userfile"
			IWebBrowser2* br = docPrivate_->browserPrivate_->getBrowserInterface();
			/*CppWebBrowser->ControlInterface*/
			br->ExecWB(OLECMDID_PASTE, OLECMDEXECOPT_DODEFAULT, &comV, &res);


			click();
			/*input->put_readOnly((FALSE));
			if ( !SUCCEEDED( inputFile->put_value(CComBSTR(IuCoreUtils::Utf8ToWstring(value).c_str())) ) ) {
				LOG(WARNING) << "setValue: IHTMLInputFileElement::setValue failed.";
			}*/
			return;
		}

		input->put_value(CComBSTR(IuCoreUtils::Utf8ToWstring(value).c_str()));
	}
}
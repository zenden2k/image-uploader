/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@yandex.ru)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include "HtmlElementPrivate_win.h"
#include "Func/WinUtils.h"
namespace ScriptAPI {
    CComQIPtr<IHTMLElement> AccessibleToHTMLElement(IAccessible* pAccessible)
    {
        ATLASSERT(pAccessible != NULL);

        // Query for IServiceProvider interface.
        CComQIPtr<IServiceProvider> spServProvider = pAccessible;
        if (spServProvider != NULL)
        {
            // Ask the service for a IHTMLElement object.
            CComQIPtr<IHTMLElement> spHtmlElement;
            /*HRESULT hRes = */ spServProvider->QueryService(IID_IHTMLElement, IID_IHTMLElement,
                (void**)&spHtmlElement);

            return spHtmlElement;
        }

        return CComQIPtr<IHTMLElement>();
    }

void HtmlElementPrivate::setValue(const std::string& value) {
    CComQIPtr<IHTMLInputElement>  input = disp_ ? CComQIPtr<IHTMLInputElement> (disp_) : CComQIPtr<IHTMLInputElement> (elem_);
    if ( !input ) {
        LOG(WARNING) << "setValue: element is not an input.";
        return;
    }

        CComQIPtr<IHTMLInputFileElement>  inputFile = disp_ ? CComQIPtr<IHTMLInputFileElement> (disp_) : CComQIPtr<IHTMLInputFileElement> (elem_);

        if ( inputFile ) {
            accessible_ = HTMLElementToAccessible(elem_);
            if ( accessible_ ) {
                CString val = IuCoreUtils::Utf8ToWstring(value).c_str();
                
                //elem2_->focus();
                docPrivate_->browserPrivate_->webViewWindow_.fillInputFileField(val, inputFile, accessible_);
            
                //docPrivate_->browserPrivate_->webViewWindow_.fillInputFileField();
                /*IDispatchPtr parent;
                accessible_->get_accParent(&parent);
                CComQIPtr<IAccessible> parentAccessible = parent;
                long childCount = 0;
                long returnCount = 0;

                HRESULT hr = parentAccessible->get_accChildCount(&childCount);

                if (childCount != 0) {
                    CComVariant* pArray = new CComVariant[childCount];
                    hr = ::AccessibleChildren(parentAccessible, 0L, childCount, pArray, &returnCount);
                    if (!FAILED(hr)) {
                        for (int x = 0; x < returnCount; x++)
                        {
                            CComVariant vtChild = pArray[x];
                            if (vtChild.vt != VT_DISPATCH)
                                continue;

                            CComPtr<IDispatch> pDisp = vtChild.pdispVal;
                            CComQIPtr<IAccessible> pAccChild = pDisp;
                            if (!pAccChild)
                                continue;

                            CComQIPtr<IHTMLInputElement> el = AccessibleToHTMLElement(pAccChild);
                            hr = el->put_value(CComBSTR(val));
                            VARIANT v;
                            v.vt = VT_I4 ;
                            v.lVal  = CHILDID_SELF;
                            hr = pAccChild->put_accValue(v, CComBSTR(val));

                            //std::wstring name = GetName(pAccChild).data();
                            
                        }
                    }
                    delete[] pArray;
                }
                

                VARIANT v;
                v.vt = VT_I4 ;
                v.lVal  = CHILDID_SELF;
                hr = accessible_->put_accValue(v, CComBSTR(val));
                LOG(INFO) << hr;*/
                /*
                CString val = IuCoreUtils::Utf8ToWstring(value).c_str();
                WinUtils::CopyTextToClipboard(val);
                VARIANT res;
                CComVariant comV(_T("userfile"));
                // Copy Full FileName To Clipboard
                //Clipboard()->SetTextBuf(sFile.c_str());
                /*inputFile->select();
                elem2_->focus();*
                // Paste from ClipBoard to "userfile"
                IWebBrowser2* br = docPrivate_->browserPrivate_->getBrowserInterface();
                //docPrivate_->browserPrivate_->setFocus();
                
                /*CppWebBrowser->ControlInterface*
                br->ExecWB(OLECMDID_PASTE, OLECMDEXECOPT_DODEFAULT, &comV, &res);

                docPrivate_->browserPrivate_->webViewWindow_.uploadFileName_ = val;
                elem2_->focus();
                docPrivate_->browserPrivate_->webViewWindow_.view_.SendMessage(WM_KEYDOWN, VK_RETURN,0);
                //click();

                /*input->put_readOnly((FALSE));
                if ( !SUCCEEDED( inputFile->put_value(CComBSTR(IuCoreUtils::Utf8ToWstring(value).c_str())) ) ) {
                    LOG(WARNING) << "setValue: IHTMLInputFileElement::setValue failed.";
                }*
                return;*/
                return;
            }
        }

        input->put_value(CComBSTR(IuCoreUtils::Utf8ToWstring(value).c_str()));
    }

const std::string HtmlElementPrivate::getValue()
{
    CComQIPtr<IHTMLInputElement>  input = disp_ ? CComQIPtr<IHTMLInputElement> (disp_) : CComQIPtr<IHTMLInputElement> (elem_);
    if ( !input ) {
        LOG(WARNING) << "getValue: element is not an input.";
        return std::string();
    }

    CComBSTR res;
    if ( SUCCEEDED( input->get_value(&res) )  && res  ) {
        return IuCoreUtils::WstringToUtf8((OLECHAR*)res);
    }
    return std::string();
}

CComQIPtr<IAccessible> HTMLElementToAccessible(IHTMLElement* pHtmlElement)
{
    ATLASSERT(pHtmlElement != NULL);

    // Query for IServiceProvider interface.
    CComQIPtr<IServiceProvider> spServProvider = pHtmlElement;
    if (spServProvider != NULL)
    {
        // Ask the service for a IAccessible object.
        CComQIPtr<IAccessible> spAccessible;
        /*HRESULT hRes = */spServProvider->QueryService(IID_IAccessible, IID_IAccessible,
            (void**)&spAccessible);

        return spAccessible;
    }

    return CComQIPtr<IAccessible>();
}

}
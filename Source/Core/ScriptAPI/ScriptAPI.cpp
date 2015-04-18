/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2015 Sergey Svistunov (zenden2k@gmail.com)

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

#include "ScriptAPI.h"
#ifdef _WIN32
#include "WebBrowser.h"
#include "HtmlDocument.h"
#include "HtmlElement.h"
#endif
#include "RegularExpression.h"
#include <Core/Network/NetworkClient.h>
#include <Core/Utils/SimpleXml.h>
#include <Core/Upload/UploadEngine.h>
#include <Core/Upload/FolderList.h>
#include <Core/Squirrelnc.h>
#include <Core/Upload/ServerSync.h>


/*
DECLARE_INSTANCE_TYPE(NetworkClient);
DECLARE_INSTANCE_TYPE(SimpleXml);
DECLARE_INSTANCE_TYPE(SimpleXmlNode);
DECLARE_INSTANCE_TYPE(ServerSettingsStruct);
DECLARE_INSTANCE_TYPE(CFolderList);
DECLARE_INSTANCE_TYPE(CFolderItem);
DECLARE_INSTANCE_TYPE(CIUUploadParams);*/

namespace ScriptAPI {;

void RegisterNetworkClientClass(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
    vm.GetRootTable().Bind("NetworkClient", Class<NetworkClient>(vm.GetVM()).
        Func("doGet", &NetworkClient::doGet).
        Func("responseBody", &NetworkClient::responseBody).
        Func("responseCode", &NetworkClient::responseCode).
        Func("setUrl", &NetworkClient::setUrl).
        Func("doPost", &NetworkClient::doPost).
        Func("addQueryHeader", &NetworkClient::addQueryHeader).
        Func("addQueryParam", &NetworkClient::addQueryParam).
        Func("addQueryParamFile", &NetworkClient::addQueryParamFile).
        Func("responseHeaderCount", &NetworkClient::responseHeaderCount).
        Func("urlEncode", &NetworkClient::urlEncode).
        Func("errorString", &NetworkClient::errorString).
        Func("doUpload", &NetworkClient::doUpload).
        Func("setMethod", &NetworkClient::setMethod).
        Func("setCurlOption", &NetworkClient::setCurlOption).
        Func("setCurlOptionInt", &NetworkClient::setCurlOptionInt).
        Func("doUploadMultipartData", &NetworkClient::doUploadMultipartData).
        Func("enableResponseCodeChecking", &NetworkClient::enableResponseCodeChecking).
        Func("setChunkSize", &NetworkClient::setChunkSize).
        Func("setChunkOffset", &NetworkClient::setChunkOffset).
        Func("setUserAgent", &NetworkClient::setUserAgent).
        Func("responseHeaderText", &NetworkClient::responseHeaderText).
        Func("responseHeaderByName", &NetworkClient::responseHeaderByName).
        Func("setReferer", &NetworkClient::setReferer));
}

void RegisterSimpleXmlClass(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
     vm.GetRootTable().Bind("SimpleXml", Class<SimpleXml>(vm.GetVM()).
         Func("LoadFromFile", &SimpleXml::LoadFromFile).
         Func("LoadFromString", &SimpleXml::LoadFromString).
         Func("SaveToFile", &SimpleXml::SaveToFile).
         Func("ToString", &SimpleXml::ToString).
         Func("GetRoot", &SimpleXml::getRoot)
     );

    vm.GetRootTable().Bind("SimpleXmlNode", Class<SimpleXmlNode>(vm.GetVM()).
        Func("Attribute", &SimpleXmlNode::Attribute).
        Func("AttributeInt", &SimpleXmlNode::AttributeInt).
        Func("AttributeBool", &SimpleXmlNode::AttributeBool).
        Func("Name", &SimpleXmlNode::Name).
        Func("Text", &SimpleXmlNode::Text).
        Func("CreateChild", &SimpleXmlNode::CreateChild).
        Func("GetChild", &SimpleXmlNode::GetChild).
        Func("SetAttribute", &SimpleXmlNode::SetAttributeString).
        Func("SetAttributeInt", &SimpleXmlNode::SetAttributeInt).
        Func("SetAttributeBool", &SimpleXmlNode::SetAttributeBool).
        Func("SetText", &SimpleXmlNode::SetText).
        Func("IsNull", &SimpleXmlNode::IsNull).
        Func("DeleteChilds", &SimpleXmlNode::DeleteChilds).
        Func("GetChildCount", &SimpleXmlNode::GetChildCount).
        Func("GetChildByIndex", &SimpleXmlNode::GetChildByIndex).
        Func("GetAttributeCount", &SimpleXmlNode::GetAttributeCount)
    );
}

void RegisterUploadClasses(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
    RootTable& root = vm.GetRootTable();
    root.Bind("CFolderItem", Class<CFolderItem>(vm.GetVM()).
        Func("getId", &CFolderItem::getId).
        Func("getParentId", &CFolderItem::getParentId).
        Func("getSummary", &CFolderItem::getSummary).
        Func("getTitle", &CFolderItem::getTitle).
        Func("setId", &CFolderItem::setId).
        Func("setParentId", &CFolderItem::setParentId).
        Func("setSummary", &CFolderItem::setSummary).
        Func("setTitle", &CFolderItem::setTitle).
        Func("getAccessType", &CFolderItem::getAccessType).
        Func("setAccessType", &CFolderItem::setAccessType).
        Func("setItemCount", &CFolderItem::setItemCount).
        Func("setViewUrl", &CFolderItem::setViewUrl).
        Func("getItemCount", &CFolderItem::getItemCount)
    );

    root.Bind("CIUUploadParams", Class<CIUUploadParams>(vm.GetVM()).
        Func("getFolderID", &CIUUploadParams::getFolderID).
        Func("setDirectUrl", &CIUUploadParams::setDirectUrl).
        Func("setThumbUrl", &CIUUploadParams::setThumbUrl).
        Func("getServerFileName", &CIUUploadParams::getServerFileName).
        Func("setViewUrl", &CIUUploadParams::setViewUrl).
        Func("getParam", &CIUUploadParams::getParam)
    );


    root.Bind("CFolderList", Class<CFolderList>(vm.GetVM()).
        Func("AddFolder", &CFolderList::AddFolder).
        Func("AddFolderItem", &CFolderList::AddFolderItem)
    );


    root.Bind("ServerSettingsStruct", Class<ServerSettingsStruct>(vm.GetVM()).
        Func("setParam", &ServerSettingsStruct::setParam).
        Func("getParam", &ServerSettingsStruct::getParam)
    );

	root.Bind("ServerSync", Class<ServerSync>(vm.GetVM()).
		Func("beginLogin", &ServerSync::beginLogin).
		Func("endLogin", &ServerSync::endLogin).
		Func("setValue", &ServerSync::setValue).
		Func("getValue", &ServerSync::getValue)
	);

}

#ifdef _MSC_VER
__declspec(thread) Sqrat::SqratVM* threadVm;
#else
thread_local  Sqrat::SqratVM* threadVm;
#endif

void RegisterClasses(Sqrat::SqratVM& vm) {
   // Sqrat::DefaultVM::Set(vm.GetVM());
    RegisterNetworkClientClass(vm);
    RegisterRegularExpressionClass(vm);
    RegisterUploadClasses(vm);
#ifdef _WIN32
	RegisterWebBrowserClass(vm);
	RegisterHtmlDocumentClass(vm);
	RegisterHtmlElementClass(vm);
#endif
}
void RegisterAPI(Sqrat::SqratVM& vm)
{
	threadVm = &vm;
    RegisterFunctions(vm);
    RegisterClasses(vm);
}
void CleanUp()
{
    CleanUpFunctions();
}

Sqrat::SqratVM& GetCurrentThreadVM()
{
	return *threadVm;
}
void SetCurrentThreadVM(Sqrat::SqratVM& vm) {
	threadVm = &vm;
}
}

HSQUIRRELVM GetCurrentThreadHVM()
{
	return ScriptAPI::GetCurrentThreadVM().GetVM();
}


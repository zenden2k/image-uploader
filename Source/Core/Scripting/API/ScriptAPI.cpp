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
#include "WebBrowserPrivateBase.h"
#endif
#include "RegularExpression.h"
#include "Process.h"
#include "Core/Network/NetworkClient.h"
#include "Core/Utils/SimpleXml.h"
#include "Core/Upload/UploadEngine.h"
#include "Core/Upload/FolderList.h"
#include "Core/Scripting/Squirrelnc.h"
#include "Core/Upload/ServerSync.h"
#include "Core/Scripting/Squirrelnc.h"
#include <set>
#include <unordered_map>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace ScriptAPI {;

std::unordered_map<HSQUIRRELVM, std::string> squirrelOutput;
std::mutex squirrelOutputMutex;
std::unordered_map<HSQUIRRELVM, PrintCallback> printCallbacks;
std::mutex printCallbacksMutex;
std::unordered_map<HSQUIRRELVM, std::string> scriptNames;
std::mutex scriptNamesMutex;

#ifndef _MSC_VER
int _vscprintf(const char * format, va_list pargs) {
    int retval;
    va_list argcopy;
    va_copy(argcopy, pargs);
    retval = vsnprintf(NULL, 0, format, argcopy);
    va_end(argcopy);
    return retval;
}
#endif
static void printFunc(HSQUIRRELVM v, const SQChar* s, ...)
{
    std::lock_guard<std::mutex> lock(squirrelOutputMutex);
    va_list vl;
    va_start(vl, s);
    int len = /*1024; /*/ _vscprintf(s, vl) + 1;
    char* buffer = new char[len + 1];
    vsnprintf(buffer, len, s, vl);
    va_end(vl);
    // std::wstring text =  Utf8ToWstring(buffer);
    squirrelOutput[v] += buffer;
    delete[] buffer;
}

// Actually we do not need this since migrating to Sqrat
void CompilerErrorHandler(HSQUIRRELVM, const SQChar * desc, const SQChar * source, SQInteger line, SQInteger column) {
    LOG(ERROR) << "Script compilation failed\r\n" << "File:  " << source << "\r\nLine:" << line << "   Column:" << column << "\r\n\r\n" << desc;
}

void RegisterNetworkClientClass(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
    vm.GetRootTable().Bind("NetworkClient", Class<NetworkClient>(vm.GetVM(), "NetworkClient").
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
        Func("getCurlInfoString", &NetworkClient::getCurlInfoString).
        Func("getCurlInfoInt", &NetworkClient::getCurlInfoInt).
        Func("getCurlInfoDouble", &NetworkClient::getCurlInfoDouble).
        Func("setReferer", &NetworkClient::setReferer));
}

void RegisterSimpleXmlClass(Sqrat::SqratVM& vm) {
    using namespace Sqrat;
     vm.GetRootTable().Bind("SimpleXml", Class<SimpleXml>(vm.GetVM(), "SimpleXml").
         Func("LoadFromFile", &SimpleXml::LoadFromFile).
         Func("LoadFromString", &SimpleXml::LoadFromString).
         Func("SaveToFile", &SimpleXml::SaveToFile).
         Func("ToString", &SimpleXml::ToString).
         Func("GetRoot", &SimpleXml::getRoot)
     );

    vm.GetRootTable().Bind("SimpleXmlNode", Class<SimpleXmlNode>(vm.GetVM(), "SimpleXmlNode").
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
    root.Bind("CFolderItem", Class<CFolderItem>(vm.GetVM(), "CFolderItem").
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

    root.Bind("CIUUploadParams", Class<UploadParams>(vm.GetVM(), "CIUUploadParams").
        Func("getFolderID", &UploadParams::getFolderID).
        Func("setDirectUrl", &UploadParams::setDirectUrl).
        Func("getDirectUrl", &UploadParams::getDirectUrl).
        Func("setThumbUrl", &UploadParams::setThumbUrl).
        Func("getThumbUrl", &UploadParams::getThumbUrl).
        Func("setViewUrl", &UploadParams::setViewUrl).
        Func("getViewUrl", &UploadParams::getViewUrl).
        Func("setDeleteUrl", &UploadParams::setDeleteUrl).
        Func("getDeleteUrl", &UploadParams::getDeleteUrl).
        Func("setEditUrl", &UploadParams::setEditUrl).
        Func("getEditUrl", &UploadParams::getEditUrl).
        Func("getServerFileName", &UploadParams::getServerFileName).
        Func("getTask", &UploadParams::getTask).
        Func("getParam", &UploadParams::getParam)
    );


    root.Bind("CFolderList", Class<CFolderList>(vm.GetVM(), "CFolderList").
        Func("AddFolder", &CFolderList::AddFolder).
        Func("AddFolderItem", &CFolderList::AddFolderItem).
        Func("GetCount", &CFolderList::GetCount)
    );


    root.Bind("ServerSettingsStruct", Class<ServerSettingsStruct>(vm.GetVM(), "ServerSettingsStruct").
        Func("setParam", &ServerSettingsStruct::setParam).
        Func("getParam", &ServerSettingsStruct::getParam)
    );

    Class<ThreadSync> threadSyncClass(vm.GetVM(), "ThreadSync");
    threadSyncClass.Func("setValue", &ThreadSync::setValue).
        Func("getValue", &ThreadSync::getValue);

    root.Bind("ServerSync", DerivedClass<ServerSync, ThreadSync>(vm.GetVM(), "ServerSync").
        Func("beginAuth", &ServerSync::beginAuth).
        Func("endAuth", &ServerSync::endAuth).
        Func("isAuthPerformed", &ServerSync::isAuthPerformed).
        Func("setAuthPerformed", &ServerSync::setAuthPerformed)
    );

}

#ifdef _MSC_VER
__declspec(thread) Sqrat::SqratVM* threadVm;
#else
thread_local  Sqrat::SqratVM* threadVm;
#endif

std::unordered_map<HSQUIRRELVM, std::set<WebBrowserPrivateBase*>> vmBrowsers;
std::mutex vmBrowsersMutex;

void RegisterClasses(Sqrat::SqratVM& vm) {
   // Sqrat::DefaultVM::Set(vm.GetVM());
    RegisterNetworkClientClass(vm);
    RegisterRegularExpressionClass(vm);
    RegisterUploadClasses(vm);
    RegisterUploadTaskWrappers(vm);
    RegisterProcessClass(vm);
#ifdef _WIN32
    RegisterWebBrowserClass(vm);
    RegisterHtmlDocumentClass(vm);
    RegisterHtmlElementClass(vm);
#endif
}
void RegisterAPI(Sqrat::SqratVM& vm)
{
    threadVm = &vm;
    //sq_setcompilererrorhandler(vm_.GetVM(), CompilerErrorHandler);
    sq_setprintfunc(vm.GetVM(), printFunc, printFunc);
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

void StopAssociatedBrowsers(Sqrat::SqratVM& vm)
{
    std::lock_guard<std::mutex> guard(vmBrowsersMutex);
    for ( auto& it : vmBrowsers[vm.GetVM()])
    {
        it->abort();
    }
    
}

void AddBrowserToVM(Sqrat::SqratVM& vm, WebBrowserPrivateBase* browser)
{
    std::lock_guard<std::mutex> guard(vmBrowsersMutex);
    vmBrowsers[vm.GetVM()].insert(browser);
}

void RemoveBrowserToVM(Sqrat::SqratVM& vm, WebBrowserPrivateBase* browser)
{
    try
    {
        std::lock_guard<std::mutex> guard(vmBrowsersMutex);
        vmBrowsers[vm.GetVM()].erase(browser);
    } catch (std::exception& ex)
    {
        LOG(ERROR) << ex.what();
    }

}

void SetPrintCallback(Sqrat::SqratVM& vm, const PrintCallback& callback)
{
    std::lock_guard<std::mutex> guard(printCallbacksMutex);
    printCallbacks[vm.GetVM()] = callback;
}

void SetScriptName(Sqrat::SqratVM& vm, const std::string& fileName)
{
    std::lock_guard<std::mutex> lock(scriptNamesMutex);
    scriptNames[vm.GetVM()] = fileName;
}

void ClearVmData(Sqrat::SqratVM& vm)
{
    std::lock_guard<std::mutex> lock(scriptNamesMutex);
    auto it = scriptNames.find(vm.GetVM());
    if (it != scriptNames.end())
    {
        scriptNames.erase(it);
    }
}

const std::string GetScriptName(HSQUIRRELVM vm)
{
    std::lock_guard<std::mutex> lock(scriptNamesMutex);
    return scriptNames[vm];
}

void FlushSquirrelOutput(Sqrat::SqratVM& vm) {
    std::lock_guard<std::mutex> guard(squirrelOutputMutex);
    std::string& output = squirrelOutput[vm.GetVM()];
    std::lock_guard<std::mutex> guard2(printCallbacksMutex);
    
    if (!output.empty())
    {
        auto it = printCallbacks.find(vm.GetVM());
        if (it != printCallbacks.end())
        {
            PrintCallback& callback = it->second;
            if (callback)
            {
                callback(output);
            }
            
        }
        output.clear();
    }
}

}

HSQUIRRELVM GetCurrentThreadHVM()
{
    return ScriptAPI::GetCurrentThreadVM().GetVM();
}


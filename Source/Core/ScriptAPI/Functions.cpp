#include "Functions.h"

#include "Core/AppParams.h"
#include <Core/Utils/CoreUtils.h>
#include <Core/Squirrelnc.h>
#include <Core/Logging.h>
#include <json/json.h>
#include <Core/Utils/StringUtils.h>
#include "versioninfo.h"

#ifndef IU_CLI
#include <Func/LangClass.h>
#endif

using namespace SqPlus;
using namespace ScriptAPI;


namespace ScriptAPI {

SquirrelObject* RootTable = 0;

const std::string GetScriptsDirectory()
{
	return AppParams::instance()->settingsDirectory() + "Scripts/";
}

const std::string GetAppLanguageFile()
{
	std::string languageFile = AppParams::instance()->languageFile();
	if ( languageFile.empty() ) {
		return "English";
	}
	return IuCoreUtils::ExtractFileNameNoExt(languageFile);
}

SquirrelObject GetAppVersion() {
	SquirrelObject res = SquirrelVM::CreateTable();
	std::string ver = _APP_VER;
	std::vector<std::string> tokens;
	IuStringUtils::Split(ver,".", tokens, 3);
	if ( tokens.size() >=3 ) {
		res.SetValue("Major", (int)IuCoreUtils::stringToint64_t(tokens[0]));
		res.SetValue("Minor", (int)IuCoreUtils::stringToint64_t(tokens[1]));
		res.SetValue("Patch", (int)IuCoreUtils::stringToint64_t(tokens[2]));
		res.SetValue("Build", (int)IuCoreUtils::stringToint64_t(BUILD));
		bool isGui = 
#ifndef IU_CLI
			true;
#else 
			false;
#endif
		res.SetValue("Gui",isGui);
	}
	return res;
}

SquirrelObject IncludeScript(const std::string& filename)
{	
	if ( filename.empty() ) {
		LOG(ERROR) << "include() failed: empty file name";
		return SquirrelObject();
	}
	std::string absolutePath;
	if ( filename[0] == '/' || (filename.length()>1 && filename[1]==':' ) ) {
		absolutePath = filename;
	} else {
		absolutePath = GetScriptsDirectory() + filename;
	}

	if ( !IuCoreUtils::FileExists(absolutePath) ) {
		LOG(ERROR) << "include() failed: file \"" + absolutePath + "\" not found.";
		return SquirrelObject();
	}
	std::string scriptText;
	if ( !IuCoreUtils::ReadUtf8TextFile(absolutePath, scriptText) ) {
		LOG(ERROR) << "include() failed: could not read file \"" + absolutePath + "\".";
		return SquirrelObject();
	}
	SquirrelObject script = SquirrelVM::CompileBuffer(scriptText.c_str(), IuCoreUtils::ExtractFileName(absolutePath).c_str());
	return SquirrelVM::RunScript(script, RootTable);
}

Json::Value* translationRoot = 0;

void CleanUpFunctions() {
	delete translationRoot;
}

bool LoadScriptTranslation() {
	if ( !translationRoot ) {
		translationRoot = new Json::Value();
		std::string absolutePath = GetScriptsDirectory() + "Lang/" + GetAppLanguageFile() + ".json";
		std::string jsonText;
		if ( !IuCoreUtils::ReadUtf8TextFile(absolutePath, jsonText) ) {
			return false;
		}
		
		Json::Reader reader;
		if ( reader.parse(jsonText, *translationRoot, false) ) {
			return true;
		}
		return false;
	} else {
		return true;
	}
}

const std::string Translate(const std::string& key, const std::string& originalText) {
	if ( LoadScriptTranslation() ) {
		std::vector<std::string> tokens;
		IuStringUtils::Split(key, ".", tokens, -1);
		Json::Value root = *translationRoot;
		int count = tokens.size();
		for ( int i = 0; i < count; i++ ) {
			std::string token = tokens[i];
			if ( !root.isMember(token) ) {
				break;
			}
			root = root[token];
			if ( root.type() != Json::objectValue && i+1 != count ) {
				break;
			}
			if ( i+1 == count && root.type() == Json::stringValue  ) {
				return root.asString();
			}
		}
	}

#ifndef IU_CLI
	return IuCoreUtils::WstringToUtf8((LPCTSTR)Lang.GetString(IuCoreUtils::Utf8ToWstring(originalText).c_str()));
#endif
	return originalText;
}

void RegisterFunctions(SquirrelObject* rootTable)
{
	RootTable = rootTable;
	RegisterGlobal(GetScriptsDirectory, "GetScriptsDirectory");
	RegisterGlobal(GetAppLanguageFile, "GetAppLanguageFile");
	RegisterGlobal(IncludeScript, "include");
	RegisterGlobal(Translate, "Translate");
	RegisterGlobal(GetAppVersion, "GetAppVersion");
	atexit(&CleanUpFunctions);
}

void RegisterShortTranslateFunctions(bool tr, bool underscore)
{
	if ( underscore ) {
		RegisterGlobal(Translate, "__");
	}
	if ( tr ) {
		RegisterGlobal(Translate, "tr");
	}
	
}

}
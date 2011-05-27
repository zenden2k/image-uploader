#ifndef IU_OUTPUTCODEGENERATOR_H
#define IU_OUTPUTCODEGENERATOR_H

#include <string>
#include <vector>
#include "Utils/CoreTypes.h"

struct ZUploadObject
{
   std::string localFilePath;
   std::string directUrl;
   std::string viewUrl;
   std::string thumbUrl;
   std::string serverName;
   std::string displayFileName;
   time_t timeStamp;
   zint64 uploadFileSize;
};

class ZOutputCodeGenerator
{

	public:
		enum CodeLang {clHTML, clBBCode, clPlain};
		enum CodeType {ctTableOfThumbnails, ctClickableThumbnails, ctImages, ctLinks};
		ZOutputCodeGenerator();
		void setLang(CodeLang lang);
		void setType(CodeType type);
		void setPreferDirectLinks(bool prefer);
      std::string generate(const std::vector<ZUploadObject>& items);
	private:
      std::string generateCodeForItem(const ZUploadObject& item, int index);
		std::string image(std::string url);
		std::string link(std::string url, std::string body);
		CodeLang m_lang;
		CodeType m_CodeType;
		bool m_PreferDirectLinks;
};

#endif

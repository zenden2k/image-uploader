#include "TempFileDeleter.h"

#include "Core/Utils/CoreUtils.h"

TempFileDeleter::TempFileDeleter()
{
}

TempFileDeleter::~TempFileDeleter()
{
	cleanup();
}

void TempFileDeleter::addFile(const std::string& fileName)
{
	m_files.push_back(fileName);
}

bool TempFileDeleter::cleanup()
{
	for (size_t i = 0; i<m_files.size(); i++)
	{
		IuCoreUtils::RemoveFile(m_files[i]);
	}
	m_files.clear();
	return true;
}
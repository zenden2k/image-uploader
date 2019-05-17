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

#include "HistoryManager.h"

#include <time.h>
#include <algorithm>
#include <boost/format.hpp>

#include <boost/filesystem.hpp>
#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Utils/GlobalMutex.h"
#include "Core/3rdpart/pcreplusplus.h"

class CHistoryReader_impl
{
    public:
        SimpleXml m_xml;
        std::vector<CHistorySession*> m_sessions;
        std::map<std::string, int> keyToIndex_;
        //sqlite3* db_;
        CHistoryManager* mgr_;
};

const char CHistoryManager::globalMutexName[] = "IuHistoryFileSessionMutex";

CHistoryManager::CHistoryManager() : db_(nullptr)
{
    m_historyFileNamePrefix = "history";
}

CHistoryManager::~CHistoryManager()
{
    if (db_) {
        sqlite3_close(db_);
    }
}

void CHistoryManager::setHistoryDirectory(const std::string& directory) {

    m_historyFilePath = directory;
    IuCoreUtils::createDirectory(m_historyFilePath);
    if (sqlite3_open((directory + "history.db").c_str(), &db_) != SQLITE_OK) {
        LOG(ERROR) << "unable to open database: ";
    } else {
        const char* sql = "CREATE TABLE IF NOT EXISTS upload_sessions(id,created_at)";
        char *err = nullptr;
        if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: " << err;
            sqlite3_free(err);
        }
        const char* sql2 = "CREATE TABLE IF NOT EXISTS uploads(`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,session_id,created_at INTEGER,local_file_path,server_name,direct_url,thumb_url,"
                            "view_url,direct_url_shortened,view_url_shortened, edit_url, delete_url, display_name, size INTEGER, sort_index INTEGER)";

        if (sqlite3_exec(db_, sql2, nullptr, nullptr, &err) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: " << err;
            sqlite3_free(err);
        }
    }   
}

bool CHistoryManager::saveSession(CHistorySession* session) {
    if (session->dbEntryCreated_) {
        return true;
    }
    IuCoreUtils::ZGlobalMutex mutex(CHistoryManager::globalMutexName);
    const char* sql = "INSERT INTO upload_sessions(id,created_at) VALUES(?,?)";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: Could not prepare statement.";
        return false;
    } else {
        std::string id = session->sessionId();
        if (sqlite3_bind_text(stmt, 1 /*Index of wildcard*/, id.c_str(), -1, nullptr) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind value.";
            return false;
        }
        if (sqlite3_bind_int(stmt, 2, session->timeStamp()) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind value.";
            return false;
        }
        /*if (sqlite3_bind_text(stmt, 3 , serverName.c_str(), -1, nullptr) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: Could not bind value.";
        }*/
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            LOG(ERROR) << "SQL error: Could not execute statement";
        }
        sqlite3_reset(stmt);
        session->dbEntryCreated_ = true;
    }
    return true;
}

bool CHistoryManager::saveHistoryItem(HistoryItem* ht) {
    saveSession(ht->session);
    IuCoreUtils::ZGlobalMutex mutex(CHistoryManager::globalMutexName);

    const char* sql = "INSERT INTO uploads(session_id,created_at,local_file_path,server_name,direct_url,thumb_url,"
        "view_url,direct_url_shortened,view_url_shortened, edit_url, delete_url, display_name, size, sort_index) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?); ";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: Could not prepare statement." << sqlite3_errmsg(db_);
        ;
        return false;
    } else {
        bindString(stmt, 1, ht->session->sessionId());
        if (sqlite3_bind_int(stmt, 2 /*Index of wildcard*/, ht->timeStamp) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind value.";
            return false;
        }
        bindString(stmt, 3, ht->localFilePath);
        bindString(stmt, 4, ht->serverName);
        bindString(stmt, 5, ht->directUrl);
        bindString(stmt, 6, ht->thumbUrl);
        bindString(stmt, 7, ht->viewUrl);
        bindString(stmt, 8, ht->directUrlShortened);
        bindString(stmt, 9, ht->viewUrlShortened);
        bindString(stmt, 10, ht->editUrl);
        bindString(stmt, 11, ht->deleteUrl);
        bindString(stmt, 12, ht->displayName);

        if (sqlite3_bind_int64(stmt, 13 /*Index of wildcard*/, ht->uploadFileSize) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind value.";
            return false;
        }
        if (sqlite3_bind_int(stmt, 14 /*Index of wildcard*/, ht->sortIndex) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind value.";
            return false;
        }
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            LOG(ERROR) << "SQL error: Could not execute statement";
            return false;
        }
        sqlite3_reset(stmt);
    }
    return true;
}

void CHistoryManager::setHistoryFileName(const std::string& filepath, const std::string& nameprefix)
{
    m_historyFilePath = filepath;
    IuCoreUtils::createDirectory(m_historyFilePath);
    m_historyFileNamePrefix = nameprefix;
}

std::shared_ptr<CHistorySession> CHistoryManager::newSession()
{
    time_t t = time(nullptr);
    tm * timeinfo = localtime ( &t );
    std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + IuCoreUtils::toString(1900+timeinfo->tm_year)+"_" + IuCoreUtils::toString(timeinfo->tm_mon+1) + ".xml";
    std::string str = IuCoreUtils::toString(rand()%(256 * 256)) + IuCoreUtils::toString(int(t));
    std::string id = IuCoreUtils::CryptoUtils::CalcMD5HashFromString(str + IuCoreUtils::toString(rand()%(256))).substr(0, 16);
    auto res = std::make_shared<CHistorySession>(db_, fileName, id);
    res->setTimeStamp(t);
    return res;
}

CHistorySession::CHistorySession(sqlite3* db, const std::string& filename, const std::string&  sessionId) : db_(db)
{
    m_historyXmlFileName = filename;
    m_sessId = sessionId;
    time(&m_timeStamp);
    dbEntryCreated_ = false;
}

void CHistorySession::loadFromXml(SimpleXmlNode& sessionNode)
{
    m_timeStamp = sessionNode.AttributeInt("TimeStamp" );
    m_serverName = sessionNode.Attribute("ServerName" );
    m_sessId = sessionNode.Attribute("ID");
    std::vector<SimpleXmlNode> allEntries;
    sessionNode.GetChilds("Entry", allEntries);

    for(size_t i=0; i<allEntries.size(); i++)
    {
        HistoryItem ht(this);
        ht.localFilePath = allEntries[i].Attribute("LocalFilePath");
        ht.serverName = allEntries[i].Attribute("ServerName");
        ht.timeStamp = allEntries[i].AttributeInt("TimeStamp" );
        ht.directUrl = allEntries[i].Attribute("DirectUrl" );
        ht.thumbUrl = allEntries[i].Attribute("ThumbUrl");
        ht.viewUrl = allEntries[i].Attribute("ViewUrl");
        ht.editUrl = allEntries[i].Attribute("EditUrl");
        ht.deleteUrl = allEntries[i].Attribute("DeleteUrl");
        ht.displayName = allEntries[i].Attribute("DisplayName");
        ht.uploadFileSize = allEntries[i].AttributeInt64("UploadFileSize");
        std::string sortIndex = allEntries[i].Attribute("Index");
        ht.sortIndex = sortIndex.empty() ? i : IuCoreUtils::stringToInt64(sortIndex);
        // Fix invalid file size
        if ( ht.uploadFileSize > 1000000000000 || ht.uploadFileSize < 0 ){
             ht.uploadFileSize  = 0;
        }
        m_entries.push_back(ht);
    }
}

int CHistorySession::entriesCount() const
{
    return m_entries.size();
}

HistoryItem CHistorySession::entry(const int index) const
{
    return m_entries[index];
}

bool CHistoryManager::bindString(sqlite3_stmt* stmt, int index,const std::string& val) {
    if (!val.empty()) {
        char* str = new char[val.size()+1];
        strcpy_s(str, val.size() + 1, val.c_str());
        if (sqlite3_bind_text(stmt, index /*Index of wildcard*/, str, -1, [](void* s) { delete[] reinterpret_cast<char*>(s); }) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind value.";
            return false;
        }
    } else {
        if (sqlite3_bind_null(stmt, index ) != SQLITE_OK) {
            LOG(ERROR) << "SQL error: Could not bind null value.";
            return false;
        }
    }
    return true;
}

void CHistorySession::sortByOrderIndex() {
    std::sort(m_entries.begin(), m_entries.end(), [](const HistoryItem& lhs, const HistoryItem& rhs) {
        return lhs.sortIndex < rhs.sortIndex;
    });
}

std::string CHistorySession::serverName() const
{
    return m_serverName;
}

void CHistorySession::setServerName(const std::string& name) {
    m_serverName = name;
}

time_t CHistorySession::timeStamp() const
{
    return m_timeStamp;
}

void CHistorySession::setTimeStamp(time_t timeStamp) {
    m_timeStamp = timeStamp;
}

std::string  CHistorySession::sessionId() const {
    return m_sessId;
}

std::vector<HistoryItem>::iterator CHistorySession::begin() {
    return m_entries.begin();
}

std::vector<HistoryItem>::iterator CHistorySession::end() {
    return m_entries.end();
}

/*std::string CHistoryManager::makeFileName() const
{
    time_t t = time(0);
    tm * timeinfo = localtime(&t);
    std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + IuCoreUtils::toString(1900+timeinfo->tm_year)+"_" + IuCoreUtils::toString(timeinfo->tm_mon+1) + ".xml";
    return fileName;
}*/

bool CHistoryManager::clearHistory(HistoryClearPeriod period) {
    IuCoreUtils::ZGlobalMutex mutex(globalMutexName);
    std::string historyFolder = m_historyFilePath;
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end

    pcrepp::Pcre regexp("^history_(\\d+)_(\\d+)\\.xml$");
    time_t t = time(nullptr);
    tm * timeinfo = localtime(&t);
    int currentMonth = timeinfo->tm_mon + 1;
    int currentYear = 1900 + timeinfo->tm_year;

    for (boost::filesystem::directory_iterator i(historyFolder); i != end_itr; ++i) {
        // Skip if not a file
        if (!boost::filesystem::is_regular_file(i->status())) {
            continue;
        }

        // Skip if no match
        if (!regexp.search(i->path().filename().string())) {
            continue;
        }
        int year = atoi(regexp[1].c_str());
        int month = atoi(regexp[2].c_str());
        if (period == HistoryClearPeriod::ClearAll || (period == HistoryClearPeriod::CurrentMonth && month == currentMonth && year == currentYear)) {
            std::string fileName = i->path().string();
            bool res = IuCoreUtils::RemoveFile(fileName);
            if (!res) {
                LOG(ERROR) << "Unable to delete file " << fileName;
            }
        }
    }
    return true;
}

bool CHistoryManager::convertHistory() {
    IuCoreUtils::ZGlobalMutex mutex(globalMutexName);
    std::string historyFolder = m_historyFilePath;
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end

    pcrepp::Pcre regexp("^history_(\\d+)_(\\d+)\\.xml$");

    for (boost::filesystem::directory_iterator i(historyFolder); i != end_itr; ++i) {
        // Skip if not a file
        if (!boost::filesystem::is_regular_file(i->status())) {
            continue;
        }

        // Skip if no match
        if (!regexp.search(i->path().filename().string())) {
            continue;
        }
        int year = atoi(regexp[1].c_str());
        int month = atoi(regexp[2].c_str());
        
        std::string fileName = i->path().string();

        CHistoryReader reader(this);
        if (!reader.loadFromFile(fileName)) {
            LOG(ERROR) << "Failed to convert history file " << fileName;
            continue;
        }


        for (auto& session : reader) {
            for (auto& item : *session) {
                saveHistoryItem(&item);
            }
        }
        std::string newName = historyFolder + i->path().filename().string() + ".bak";
        if (!IuCoreUtils::MoveFileOrFolder(i->path().string(), newName)) {
            LOG(ERROR) << "Unable to rename file " << i->path();
        }
    }
    return true;
}

// class CHistoryReader
//
CHistoryReader::CHistoryReader(CHistoryManager* mgr)
{
    d_ptr = new CHistoryReader_impl();
    d_ptr->mgr_ = mgr;
}

int CHistoryReader::getSessionCount() const
{
    return d_ptr->m_sessions.size();
}

CHistorySession* CHistoryReader::getSession(size_t index)
{
    return d_ptr->m_sessions[index];
}

bool CHistoryReader::loadFromFile(const std::string& filename)
{
    if(IuCoreUtils::FileExists(filename))
    {
        if (!d_ptr->m_xml.LoadFromFile(filename)) {
            return false;
        }
    }
    else {
        return false;
    }
    SimpleXmlNode root = d_ptr->m_xml.getRoot("History");
    std::vector<SimpleXmlNode> allSessions;
    root.GetChilds("Session", allSessions);

    for(size_t i = 0; i<allSessions.size(); i++)
    {
        std::string fName = filename;
        CHistorySession* session = new CHistorySession(nullptr,fName, "0");
        session->loadFromXml(allSessions[i]);
        d_ptr->m_sessions.push_back(session);
    }
    return true;
}

bool CHistoryReader::loadFromDB(unsigned short year, unsigned short month) {
    sqlite3* db = d_ptr->mgr_->db_;

    std::string condition;

    if (year) {
        tm startTime;
        memset(&startTime, 0, sizeof(startTime));
        startTime.tm_year = year - 1900;
        startTime.tm_mon = month - 1;
        startTime.tm_mday = 1;

        tm endTime;

        memset(&endTime, 0, sizeof(endTime));
        endTime.tm_year = year - 1900;
        endTime.tm_mon = month;
        endTime.tm_mday = 1;

        if (month == 12) {
            endTime.tm_mon = 0;
            endTime.tm_year++;
        }
        condition = boost::str(boost::format(" where created_at between %d and %d") % mktime(&startTime) % mktime(&endTime));
    }
   
    std::string sql = "SELECT * from upload_sessions " + condition;

    char *err = nullptr;

    if (sqlite3_exec(db, sql.c_str(), selectCallback, this, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: " << err;
        sqlite3_free(err);
    }

    std::string query2 = "SELECT * from uploads " + condition;

    if (sqlite3_exec(db, query2.c_str(), selectCallback2, this, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: " << err;
        sqlite3_free(err);
        return false;
    }

    for (auto it : d_ptr->m_sessions) {
        it->sortByOrderIndex();
    }
    

    return true;
}

int CHistoryReader::selectCallback(void* userData, int argc, char **argv, char **azColName){
    auto pthis = reinterpret_cast<CHistoryReader*>(userData);
    std::map<std::string, std::string> values;

    for (int i = 0; i < argc; i++) {
        values[azColName[i]] = argv[i] ? argv[i] : std::string();
    }
    std::string sessionId = values["id"];
    CHistorySession* session = new CHistorySession(pthis->d_ptr->mgr_->db_, "", sessionId);
    session->setTimeStamp(IuCoreUtils::stringToInt64(values["created_at"]));
    session->setServerName(values["server_name"]);
    pthis->d_ptr->m_sessions.push_back(session);
    int index = pthis->d_ptr->m_sessions.size() - 1;
    pthis->d_ptr->keyToIndex_[sessionId] = index;
    return 0;
}

int CHistoryReader::selectCallback2(void* userData, int argc, char **argv, char **azColName){
    auto pthis = reinterpret_cast<CHistoryReader*>(userData);
    std::map<std::string, std::string> values;

    for (int i = 0; i < argc; i++) {
        values[azColName[i]] = argv[i] ? argv[i] : std::string();
    }
    std::string sessionId = values["session_id"];
    auto it = pthis->d_ptr->keyToIndex_.find(sessionId);
    if (it == pthis->d_ptr->keyToIndex_.end()) {
        return 0; // No session with such id
    }
    CHistorySession* session = pthis->d_ptr->m_sessions[it->second]; 
    HistoryItem item(session);
    item.id = IuCoreUtils::stringToInt64(values["id"]);

    item.directUrl = values["direct_url"];
    item.viewUrl = values["view_url"];
    item.thumbUrl = values["thumb_url"];
    item.directUrlShortened = values["direct_url_shortened"];
    item.viewUrlShortened = values["view_url_shortened"];
    item.deleteUrl = values["delete_url"];
    item.editUrl = values["edit_url"];
    item.displayName = values["display_name"];
    item.localFilePath = values["local_file_path"];
    item.timeStamp = IuCoreUtils::stringToInt64(values["created_at"]);
    item.sortIndex = IuCoreUtils::stringToInt64(values["sort_index"]);
    item.uploadFileSize = IuCoreUtils::stringToInt64(values["size"]);
    item.serverName = values["server_name"];

    session->m_entries.push_back(std::move(item));

    return 0;
}

std::vector<CHistorySession*>::iterator CHistoryReader::begin() {
    return d_ptr->m_sessions.begin();
}
std::vector<CHistorySession*>::iterator CHistoryReader::end() {
    return d_ptr->m_sessions.end();
}

CHistoryReader::~CHistoryReader()
{
    for (auto& it : d_ptr->m_sessions) {
        delete it;
    }
    delete d_ptr;
}
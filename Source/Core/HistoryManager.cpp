/*

    Image Uploader -  free application for uploading images/files to the Internet

    Copyright 2007-2018 Sergey Svistunov (zenden2k@gmail.com)

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

#include <ctime>
#include <algorithm>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <sqlite3.h>
#include "Core/Utils/SimpleXml.h"
#include "Core/Utils/CoreUtils.h"
#include "Core/Utils/CryptoUtils.h"
#include "Utils/GlobalMutex.h"
#include "Core/3rdpart/pcreplusplus.h"
#include "Utils/StringUtils.h"

class CHistoryReader_impl
{
    public:
        SimpleXml m_xml;
        std::vector<CHistorySession*> m_sessions;
        std::map<std::string, int> keyToIndex_;
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
    sqlite3_shutdown();
}

void CHistoryManager::setHistoryDirectory(const std::string& directory) {
    m_historyFilePath = directory; 
}
bool CHistoryManager::openDatabase() {
    IuCoreUtils::createDirectory(m_historyFilePath);
    if (!db_ && sqlite3_open((m_historyFilePath + "history.db").c_str(), &db_) != SQLITE_OK) {
        LOG(ERROR) << "unable to open database: ";
        return false;
    }
    sqlite3_busy_timeout(db_, 1000); // Wait 1 second before SQLITE_BUSY is returned
    const char* sql = "CREATE TABLE IF NOT EXISTS upload_sessions(id PRIMARY KEY NOT NULL,created_at INT NOT NULL)";
    char* err = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: " << err;
        sqlite3_free(err);
        return false;
    }
    const char* sql2 =
        "CREATE TABLE IF NOT EXISTS uploads(`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,session_id,created_at INTEGER,local_file_path,server_name,direct_url,thumb_url,"
        "view_url,direct_url_shortened,view_url_shortened, edit_url, delete_url, display_name, size INTEGER, sort_index INTEGER)";

    if (sqlite3_exec(db_, sql2, nullptr, nullptr, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: " << err;
        sqlite3_free(err);
        return false;
    }
    return true;
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
    }
    std::string id = session->sessionId();
    if (sqlite3_bind_text(stmt, 1 /*Index of wildcard*/, id.c_str(), -1, nullptr) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: Could not bind value.";
        return false;
    }
    if (sqlite3_bind_int64(stmt, 2, session->timeStamp()) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: Could not bind value.";
        return false;
    }
    /*if (sqlite3_bind_text(stmt, 3 , serverName.c_str(), -1, nullptr) != SQLITE_OK) {
    LOG(ERROR) << "SQL error: Could not bind value.";
    }*/
    int retCode = sqlite3_step(stmt);
    if (retCode  != SQLITE_DONE) {
        LOG(ERROR) << "SQL error: Could not execute statement, return code=" << retCode;
    }
    /*sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);*/
    sqlite3_finalize(stmt);
    session->dbEntryCreated_ = true;

    return true;
}

bool CHistoryManager::saveHistoryItem(HistoryItem* ht) {
    saveSession(ht->session);
    IuCoreUtils::ZGlobalMutex mutex(globalMutexName);

    const char* sql = "INSERT INTO uploads(session_id,created_at,local_file_path,server_name,direct_url,thumb_url,"
        "view_url,direct_url_shortened,view_url_shortened, edit_url, delete_url, display_name, size, sort_index) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?); ";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v3(db_, sql, -1, 0, &stmt, nullptr) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: Could not prepare statement." << sqlite3_errmsg(db_);
        return false;
    }
    bindString(stmt, 1, ht->session->sessionId());
    if (sqlite3_bind_int64(stmt, 2 /*Index of wildcard*/, ht->timeStamp) != SQLITE_OK) {
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
    /*sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);*/
    sqlite3_finalize(stmt);
    
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
    std::string fileName = m_historyFilePath + m_historyFileNamePrefix +"_" + std::to_string(1900+timeinfo->tm_year)+"_" + std::to_string(timeinfo->tm_mon+1) + ".xml";
    std::string str = std::to_string(rand()%(256 * 256)) + std::to_string(int(t));
    std::string id = IuCoreUtils::CryptoUtils::CalcMD5HashFromString(str + std::to_string(rand()%(256))).substr(0, 16);
    auto res = std::make_shared<CHistorySession>(fileName, id);
    res->setTimeStamp(t);
    return res;
}

CHistorySession::CHistorySession(const std::string& filename, const std::string&  sessionId) 
{
    deleteItems_ = true;
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
        SimpleXmlNode& item = allEntries[i];
        HistoryItem *ht = new HistoryItem(this);
        ht->localFilePath = item.Attribute("LocalFilePath");
        ht->serverName = item.Attribute("ServerName");
        ht->timeStamp = item.AttributeInt("TimeStamp");
        ht->directUrl = item.Attribute("DirectUrl");
        ht->thumbUrl = item.Attribute("ThumbUrl");
        ht->viewUrl = item.Attribute("ViewUrl");
        ht->editUrl = item.Attribute("EditUrl");
        ht->deleteUrl = item.Attribute("DeleteUrl");
        ht->displayName = item.Attribute("DisplayName");
        ht->uploadFileSize = item.AttributeInt64("UploadFileSize");
        std::string sortIndex = item.Attribute("Index");
        ht->sortIndex = sortIndex.empty() ? i : static_cast<int>(IuCoreUtils::stringToInt64(sortIndex));
        // Fix invalid file size
        if (ht->uploadFileSize > 1000000000000 || ht->uploadFileSize < 0) {
            ht->uploadFileSize = 0;
        }
        m_entries.push_back(ht);
    }
}

int CHistorySession::entriesCount() const
{
    return m_entries.size();
}

HistoryItem& CHistorySession::entry(int index) const
{
    return *m_entries[index];
}

void  CHistorySession::setDeleteItems(bool doDelete) {
    deleteItems_ = doDelete;
}

CHistorySession::~CHistorySession() {
    if (deleteItems_) {
        for (auto it : m_entries) {
            delete it;
        }
    }
}

bool CHistoryManager::bindString(sqlite3_stmt* stmt, int index,const std::string& val) {
    if (!val.empty()) {
        char* str = new char[val.size()+1];
        strcpy(str, val.c_str());
        if (sqlite3_bind_text(stmt, index /*Index of wildcard*/, str, -1, [](void* s)
        {
            delete[] reinterpret_cast<char*>(s);
        }) != SQLITE_OK) {
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
    std::sort(m_entries.begin(), m_entries.end(), [](const HistoryItem* lhs, const HistoryItem* rhs) {
        return lhs->sortIndex < rhs->sortIndex;
    });
    if (!m_entries.empty()) {
        m_serverName = m_entries[0]->serverName;
    }
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

std::vector<HistoryItem*>::iterator CHistorySession::begin() {
    return m_entries.begin();
}

std::vector<HistoryItem*>::iterator CHistorySession::end() {
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

    std::string condition;
    if (period == HistoryClearPeriod::OlderThan30Days) {
        time_t now;
        time(&now);
        tm* timeinfo = localtime(&now);
        IuCoreUtils::DatePlusDays(timeinfo, -30);
        time_t startTime = mktime(timeinfo);
        condition = str(boost::format(" AND created_at < %d") % startTime);
    }

    std::string sql = str(boost::format("DELETE from uploads WHERE TRUE %1% ; DELETE from upload_sessions WHERE TRUE  %1%") % condition);
    char *err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error occured while clearing history: " << std::endl << err;
        sqlite3_free(err);
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
                saveHistoryItem(item);
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
    d_ptr.reset(new CHistoryReader_impl());
    d_ptr->mgr_ = mgr;
}

int CHistoryReader::getSessionCount() const
{
    return d_ptr->m_sessions.size();
}

CHistorySession* CHistoryReader::getSession(size_t index) const
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
        CHistorySession* session = new CHistorySession(fName, "0");
        session->loadFromXml(allSessions[i]);
        d_ptr->m_sessions.push_back(session);
    }
    return true;
}

bool CHistoryReader::loadFromDB(time_t from, time_t to, const std::string& filename, const std::string& url) {
    sqlite3* db = d_ptr->mgr_->db_;

    std::string condition, condition2;

    if (from && to) {
        condition += boost::str(boost::format(" and  created_at between %d and %d") % from % to);
    }
   
    std::string sql = "SELECT * from upload_sessions WHERE true " + condition;

    char *err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), selectCallback, this, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: " << err;
        sqlite3_free(err);
    }

    if (!filename.empty()) {
        std::string preparedFilename = IuStringUtils::Replace(filename, "'", "''");
        preparedFilename = IuStringUtils::Replace(preparedFilename, "_", "\\_");
        condition2 = boost::str(boost::format(" AND(local_file_path LIKE '%%%s%%' ESCAPE '\\')")%preparedFilename);
    }

    if (!url.empty()) {
        std::string preparedURL = IuStringUtils::Replace(url, "'", "''");
        preparedURL = IuStringUtils::Replace(preparedURL, "_", "\\_");
        std::string subExpr = "FALSE ";
        std::string fields[]{
            "direct_url", "view_url", "thumb_url", "direct_url_shortened", "view_url_shortened"
        };
        for (const auto& field : fields) {
            subExpr += boost::str(boost::format(" OR %1% LIKE '%%%2%%%' ESCAPE '\\'") % field %  preparedURL);
        }
        condition2 += boost::str(boost::format(" AND(%s)") % subExpr);
    }
    std::string query2 = "SELECT * from uploads WHERE true " + condition + condition2;

    if (sqlite3_exec(db, query2.c_str(), selectCallback2, this, &err) != SQLITE_OK) {
        LOG(ERROR) << "SQL error: " << err;
        sqlite3_free(err);
        return false;
    }

    for (auto it = d_ptr->m_sessions.begin(); it != d_ptr->m_sessions.end(); ) {
        if (!(*it)->entriesCount()) {
            it = d_ptr->m_sessions.erase(it);
        } else {
            (*it)->sortByOrderIndex();
            ++it;
        }
    }
    return true;
}

int CHistoryReader::selectCallback(void* userData, int argc, char **argv, char **azColName){
    auto pthis = reinterpret_cast<CHistoryReader*>(userData);

    std::string sessionId;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(azColName[i], "id")) {
            sessionId = argv[i] ? argv[i] : "";
            break;
        }
    }
    
    CHistorySession* session = new CHistorySession("", sessionId);
    for (int i = 0; i < argc; i++) {
        const char* val = argv[i] ? argv[i] : "";
        if (!strcmp(azColName[i], "created_at")) {
            session->setTimeStamp(IuCoreUtils::stringToInt64(val));
            break;
        } else if (!strcmp(azColName[i], "server_name")) {
            session->setServerName(val);
            break;
        }
    }

    pthis->d_ptr->m_sessions.push_back(session);
    int index = pthis->d_ptr->m_sessions.size() - 1;
    pthis->d_ptr->keyToIndex_[sessionId] = index;
    return 0;
}

int CHistoryReader::selectCallback2(void* userData, int argc, char **argv, char **azColName){
   
    auto pthis = reinterpret_cast<CHistoryReader*>(userData);

    std::string sessionId;
    for (int i = 0; i < argc; i++) {
        if (!strcmp(azColName[i], "session_id")) {
            sessionId = argv[i] ? argv[i] : std::string();
            break;
        }
    }

    auto it = pthis->d_ptr->keyToIndex_.find(sessionId);
    if (it == pthis->d_ptr->keyToIndex_.end()) {
        return 0; // No session with such id
    }
    CHistorySession* session = pthis->d_ptr->m_sessions[it->second];
    HistoryItem* item = new HistoryItem(session);

    for (int i = 0; i < argc; i++) {
        const char *val = argv[i] ? argv[i] : "";
        if (!strcmp(azColName[i], "id")) {
            item->id = static_cast<int>(IuCoreUtils::stringToInt64(val));
        } else if (!strcmp(azColName[i], "direct_url")) {
            item->directUrl = val;
        } else if (!strcmp(azColName[i], "view_url")) {
            item->viewUrl = val;
        } else if (!strcmp(azColName[i], "thumb_url")) {
            item->thumbUrl = val;
        } else if (!strcmp(azColName[i], "direct_url_shortened")) {
            item->directUrlShortened = val;
        } else if (!strcmp(azColName[i], "view_url_shortened")) {
            item->viewUrlShortened = val;
        } else if (!strcmp(azColName[i], "delete_url")) {
            item->deleteUrl = val;
        } else if (!strcmp(azColName[i], "edit_url")) {
            item->editUrl = val;
        } else if (!strcmp(azColName[i], "display_name")) {
            item->displayName = val;
        } else if (!strcmp(azColName[i], "local_file_path")) {
            item->localFilePath = val;
        } else if (!strcmp(azColName[i], "created_at")) {
            item->timeStamp = IuCoreUtils::stringToInt64(val);
        } else if (!strcmp(azColName[i], "sort_index")) {
            item->sortIndex = static_cast<int>(IuCoreUtils::stringToInt64(val));
        } else if (!strcmp(azColName[i], "size")) {
            item->uploadFileSize = IuCoreUtils::stringToInt64(val);
        } else if (!strcmp(azColName[i], "server_name")) {
            item->serverName = val;
        }
    }

    session->m_entries.push_back(item);

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
}
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

#ifndef IU_CORE_HISTORY_MANAGER_H
#define IU_CORE_HISTORY_MANAGER_H

#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Core/Utils/CoreTypes.h"

class CHistorySession;

struct HistoryItem
{
    std::string localFilePath;
    std::string directUrl;
    std::string directUrlShortened;
    std::string viewUrl;
    std::string viewUrlShortened;
    std::string thumbUrl;
    std::string editUrl;
    std::string deleteUrl;
    std::string displayName;

    std::string serverName;
    time_t timeStamp;
    int64_t uploadFileSize;
    int sortIndex;
    int id;
    CHistorySession* session;

    explicit HistoryItem(CHistorySession* historySession) : session(historySession){
        timeStamp = 0;
        time(&timeStamp);
        uploadFileSize = 0;
        sortIndex = 0;
        id = 0;
    }
};

class SimpleXmlNode;
class CHistoryReader_impl;
class CHistoryReader;
struct sqlite3;
struct sqlite3_stmt;

class CHistorySession
{
    public:
        explicit CHistorySession(const std::string& filename, const std::string& sessionId);
        ~CHistorySession();
        int entriesCount() const;
        HistoryItem& entry(int index) const;
        std::string serverName() const;
        void setServerName(const std::string& name);
        time_t timeStamp() const;
        void setTimeStamp(time_t timeStamp);
        void loadFromXml(SimpleXmlNode& sessionNode);
        std::string sessionId() const;
        std::vector<HistoryItem*>::iterator begin();
        std::vector<HistoryItem*>::iterator end();
        void setDeleteItems(bool doDelete);
    private:
        std::string m_historyXmlFileName;
        std::string m_sessId;
        time_t m_timeStamp;
        std::string m_serverName;
        std::vector<HistoryItem*> m_entries;
        bool dbEntryCreated_;
        bool deleteItems_;
        void sortByOrderIndex();
        friend class CHistoryReader;
        friend class CHistoryManager;
       
};

enum class HistoryClearPeriod { ClearAll, OlderThan30Days };

class CHistoryManager
{
    public:
        CHistoryManager();
        bool openDatabase();
        virtual ~CHistoryManager();
        void setHistoryDirectory(const std::string& directory);
        void setHistoryFileName(const std::string& filepath, const std::string& nameprefix);
        std::shared_ptr<CHistorySession> newSession();
        //std::string makeFileName() const;
        bool clearHistory(HistoryClearPeriod period);
        bool saveHistoryItem(HistoryItem* item);
        bool saveSession(CHistorySession* session);
        /**
         * Load history files (*xml) into sqlite database
         */
        bool convertHistory();
        static const char globalMutexName[];
    private:
        DISALLOW_COPY_AND_ASSIGN(CHistoryManager);
        std::string m_historyFilePath;
        std::string m_historyFileNamePrefix;
        sqlite3* db_;
        bool bindString(sqlite3_stmt* stmt, int index, const std::string& val);
        friend class CHistoryReader;
};

class CHistoryReader
{
    public:
        explicit CHistoryReader(CHistoryManager* mgr);
        virtual ~CHistoryReader();
        // filename must be utf-8 encoded
        bool loadFromFile(const std::string& filename);
        bool loadFromDB(time_t from, time_t to, const std::string& filename, const std::string& url);
        int getSessionCount() const;

        // The pointer returned by this function is only valid
        //  during lifetime of CHistoryReader object
        CHistorySession* getSession(size_t index) const;

        std::vector<CHistorySession*>::iterator begin();
        std::vector<CHistorySession*>::iterator end();

    private:
        DISALLOW_COPY_AND_ASSIGN(CHistoryReader);
        std::unique_ptr<CHistoryReader_impl> d_ptr;

        static int selectCallback(void* userData, int argc, char **argv, char **azColName);
        static int selectCallback2(void* userData, int argc, char **argv, char **azColName);
};
#endif

#ifndef QIMAGEUPLOADER_GUI_MODELS_UPLOADTREEMODEL_H
#define QIMAGEUPLOADER_GUI_MODELS_UPLOADTREEMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "Core/Upload/UploadManager.h"
#include "Core/Upload/UploadTask.h"

class UploadTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:

	struct InternalItem {
		int index;
		std::shared_ptr<UploadTask> task;
		std::shared_ptr<UploadSession> session;
		InternalItem() {
			index = 0;
		}
	};

    UploadTreeModel(QObject *parent, UploadManager *uploadManager);
	 ~UploadTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
	InternalItem* getInternalItem(const QModelIndex &index);
    void reset();
private:
    void setupModelData(UploadManager *uploadManager);
    
    UploadManager *m_uploadManager;
    /*std::map<UploadTask*, InternalItem*> m_objMap;
    std::map<UploadSession*, InternalItem*> sessionsMap_;*/
    int byUploadTask(UploadTask* obj) const;
    int byUploadSession(UploadSession* obj) const;
    bool insertRow(const QModelIndex &parent, InternalItem* internalItem);
    void recalcObjMap();
    void data_OnChildAdded(UploadTask* child);
    void data_OnUploadProgress(UploadTask* task);
    void data_OnSessionAdded(UploadSession* task);
    void data_OnStatusChanged(UploadTask* it);
	
    //public slots:
    Q_INVOKABLE  void onChildAdded(UploadTask* child);
    Q_INVOKABLE  void onUploadProgress(UploadTask* task);
    Q_INVOKABLE  void onStatusChanged(UploadTask* it);
    Q_INVOKABLE  void OnSessionAdded(UploadSession* session);
 };


 #endif

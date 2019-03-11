#include "uploadtreemodel.h"

#include <QtGui>
#include <QStringList>

#include "Core/CommonDefs.h"

Q_DECLARE_METATYPE( UploadSession* )
Q_DECLARE_METATYPE( UploadTask* )

UploadTreeModel::UploadTreeModel(QObject *parent,UploadManager *uploadManager)
    : QAbstractItemModel(parent)
{
    qRegisterMetaType<UploadSession*>("UploadSession*");
    qRegisterMetaType<UploadTask*>("UploadTask*");
    setupModelData(uploadManager);
}  

UploadTreeModel::~UploadTreeModel()
{

}

int UploadTreeModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant UploadTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    InternalItem * obj = reinterpret_cast<InternalItem*>(index.internalPointer());

    if(role == Qt::DecorationRole && index.column()==0 && obj->task)
    {
        QIcon ico;
        if(obj->task->status()== UploadTask::StatusFinished)
            ico = QIcon(":/icons/res/transfer_success.png");
        else if(obj->task->status() ==  UploadTask::StatusInQueue)
            ico = QIcon(":/icons/res/transfer_inactive.png");
        else if(obj->task->status() == UploadTask::StatusRunning)
            ico = QIcon(":/icons/res/transfer.png");
        else return QVariant();
        return ico;
    }
    if (role != Qt::DisplayRole)
        return QVariant();

    if(!obj) return QVariant();

    QString result;
    if(index.column() == 0)
    {
        if ( obj->task ) {
            result = U2Q(obj->task->title());
        } else if ( obj->session) {
            result = "session " + QString::number(obj->index + 1);
        }

    }
    else if (index.column() == 1)
    {
        if ( obj->task ) {
            result = U2Q(obj->task->progress()->statusText);
        }
    }
    else if (index.column() == 2)
    {
		auto task = obj->task;
		if (task) {
			auto progress = task->progress();
			if (progress->uploaded && progress->totalUpload)
			{

				QString speedText = QString("%1 of %2").arg(progress->uploaded).arg(progress->totalUpload);
				if (progress->totalUpload)
				{
					QString perc;
					perc = QString::number((long)(100 * (double)progress->uploaded / progress->totalUpload)) + "%";
					speedText += "(" + perc + ")";
				}
				if (!progress->speed.empty())
				{
					speedText += " [" + QString::fromUtf8(progress->speed.c_str()) +"]";
				}
				result = speedText;
			}
		}
    }
    else if (index.column() == 3)
    {
        if ( obj->task ) {
            result = U2Q(obj->task->serverName());
        }
    }
    return result;
}

Qt::ItemFlags UploadTreeModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return 0;
	}

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant UploadTreeModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    QStringList headerLabels ;
    headerLabels << tr("File") << tr("Status") << tr("Progress") << tr("Server");

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headerLabels[section];
    }

    return QVariant();
}

QModelIndex UploadTreeModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}

    InternalItem * internalItem = nullptr;

    if (!parent.isValid()) {
        auto session = m_uploadManager->session(row);
		internalItem = reinterpret_cast<InternalItem*>(session->userData());
    }
    else {
        InternalItem * internalParentItem = reinterpret_cast<InternalItem*> (parent.internalPointer());
        if ( internalParentItem->task ) {
            auto task = internalParentItem->task->child(row);
			if (task) {
				internalItem = reinterpret_cast<InternalItem*>(task->userData());
			}
		}
		else if (internalParentItem->session) {
			auto task = internalParentItem->session->getTask(row);
			if (task) {
				internalItem = reinterpret_cast<InternalItem*>(task->userData());
			}
        }

    }

	if (internalItem) {
		return createIndex(row, column, internalItem);
	}
	else {
		return QModelIndex();
	}
}

QModelIndex UploadTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    /*if ( index.column() == 0 )*/ {
        InternalItem * internalItem = reinterpret_cast<InternalItem*>(index.internalPointer());

        if (internalItem->task ) {
            UploadTask* parent = internalItem->task->parentTask();
            if(!parent) {
                UploadSession* session = internalItem->task->session();
				InternalItem* item = reinterpret_cast<InternalItem*>(session->userData());
                /*auto it = sessionsMap_.find(session);
                if ( it == sessionsMap_.end() ) {
                    return QModelIndex();
                }*/
                return createIndex(item->index, 0, session->userData());
            }
            UploadTask* parentTask = internalItem->task->parentTask();
			InternalItem* item = reinterpret_cast<InternalItem*>(parentTask->userData());
            /*auto it = m_objMap.find(parentTask);
            if ( it == m_objMap.end() ) {
                return QModelIndex();
            }*/
            return createIndex(item->index, 0, parentTask->userData());
        } else {
            //UploadSession* has no parent
            return QModelIndex();
        }
        // return createIndex(upload->childCount(), 0, parent);
    }
    return QModelIndex();

}

int UploadTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        if ( parent.column() != 0) {
            return 0;
        }
        InternalItem * internalParentItem = reinterpret_cast<InternalItem*>(parent.internalPointer());

        if ( internalParentItem->task ) {
            return internalParentItem->task->childCount();
        } else {
            return internalParentItem->session->taskCount();
        }
        return 0;
    }

    //return  0;
    return m_uploadManager->sessionCount();
}

void UploadTreeModel::setupModelData(UploadManager *uploadManager)
{
    m_uploadManager = uploadManager;
    m_uploadManager->OnTaskAdded.bind(this, &UploadTreeModel::data_OnChildAdded);
    //m_uploadManager->OnChildAdded.bind(this, &UploadTreeModel::data_OnChildAdded);
    m_uploadManager->OnSessionAdded.bind(this,&UploadTreeModel::OnSessionAdded);
    //m_uploadManager->OnU.bind(this,  &UploadTreeModel::data_OnUploadProgress);
     
    /*connect(uploadManager, SIGNAL(OnChildAdded(UploadTask*,UploadTask*)), this, SLOT(data_OnChildAdded(UploadTask*,UploadTask*))/, Qt::BlockingQueuedConnection*);
    connect(uploadManager, SIGNAL(OnUploadProgress(UploadTask*,InfoProgress)), this, SLOT(data_OnUploadProgress(UploadTask*,InfoProgress)));
    connect(uploadManager, SIGNAL(OnStatusChanged(UploadTask*,QString)), this, SLOT(data_OnStatusChanged(UploadTask*,QString)));
    */
    //emit dataChanged();
}

void UploadTreeModel::data_OnChildAdded(UploadTask* child)
{
    QMetaObject::invokeMethod( this, "onChildAdded", Qt::BlockingQueuedConnection,
                               Q_ARG( UploadTask*, child ) );
}

void UploadTreeModel::data_OnUploadProgress(UploadTask* task)
{
    QMetaObject::invokeMethod( this, "onUploadProgress", Qt::BlockingQueuedConnection,
                               Q_ARG( UploadTask*, task ) );
}

void UploadTreeModel::data_OnSessionAdded(UploadSession *session) {
    QMetaObject::invokeMethod( this, "OnSessionAdded", Qt::BlockingQueuedConnection,
                               Q_ARG( UploadSession*, session ) );
}

int UploadTreeModel::byUploadTask(UploadTask* obj) const
{
	InternalItem* item = reinterpret_cast<InternalItem*>(obj->userData());
	if (item) {
		return item->index;
	}
  
    return -1;
}

int UploadTreeModel::byUploadSession(UploadSession *obj) const
{
	InternalItem* item = reinterpret_cast<InternalItem*>(obj->userData());
	if (item) {
		return item->index;
	}
    return -1;
}

void UploadTreeModel::data_OnStatusChanged(UploadTask* it)
{
    QMetaObject::invokeMethod( this, "onStatusChanged", Qt::BlockingQueuedConnection,
                               Q_ARG( UploadTask*, it ) );
}

void UploadTreeModel::onChildAdded(UploadTask *child)
{
    recalcObjMap();
    if(!child->parentTask())
    {
		UploadSession* session = child->session();
		/*auto it = sessionsMap_[session];
		auto it2 = m_objMap[child];*/
        insertRow(createIndex(byUploadSession(session), 0, session->userData()), reinterpret_cast<InternalItem*>(child->userData()));
        //insertRow(QModelIndex());

    }
    else
    {
        //insertRow(QModelIndex());
        UploadTask * parent = child->parentTask();
		//auto parentData = m_objMap[parent];
        //InternalItem* it = m_objMap[child];
		InternalItem* it = reinterpret_cast<InternalItem*>(child->userData());
        insertRow(createIndex(byUploadTask(parent), 0, parent->userData()), it);
    }
}

void UploadTreeModel::onUploadProgress(UploadTask *task)
{
	int row = byUploadTask(task);
	auto it = reinterpret_cast<InternalItem*>(task->userData());
	emit dataChanged(createIndex(row,2,it), createIndex(row,2,it));
}

void UploadTreeModel::onStatusChanged(UploadTask *task)
{
    int row = byUploadTask(task);
	auto it = reinterpret_cast<InternalItem*>(task->userData());
    emit dataChanged(createIndex(row,1,it), createIndex(row,1,it));
}

void UploadTreeModel::OnSessionAdded(UploadSession *session) {
    recalcObjMap();
	auto it = reinterpret_cast<InternalItem*>(session->userData());
    insertRow(/*createIndex(byUploadSession(session), 0, it)*/QModelIndex(), it);
	int taskCount = session->taskCount();
	for (int i = 0; i < taskCount; i++) {
		auto task = session->getTask(i);
		task->OnUploadProgress.bind(this, &UploadTreeModel::data_OnUploadProgress);
		task->OnStatusChanged.bind(this, &UploadTreeModel::data_OnStatusChanged);

	}
}

bool UploadTreeModel::insertRow( const QModelIndex &parent, InternalItem* internalItem) {
    //InternalItem* internalItem = reinterpret_cast<InternalItem*>(parent.internalPointer());

    int pos = 0;
    if ( internalItem->task ) {
		pos = internalItem->session->taskCount();
        //pos = internalItem->task->childCount();
    } else if ( internalItem->session ) {
		pos = m_uploadManager->sessionCount();
        //pos = internalItem->session->taskCount();
    } else {
        
    }

    beginInsertRows(parent,pos, pos);
    endInsertRows();
    return true;
}

void UploadTreeModel::recalcObjMap()
{
    /*sessionsMap_.clear();
    m_objMap.clear();*/
    int k = 0;
    int sessionCount = m_uploadManager->sessionCount();
    for( int i = 0; i < sessionCount; i++ ) {
        std::shared_ptr<UploadSession> session = m_uploadManager->session(i);

		InternalItem * item = reinterpret_cast<InternalItem*>(session->userData());
		if (!item) {
			item = new InternalItem();
		}
        item->session = session;
        item->index = i;
		session->setUserData(item);
        //sessionsMap_[session.get()] = item;

        int childCount = session->taskCount();
        for ( int j = 0; j < childCount; j++ ) {
			auto task = session->getTask(j);
			InternalItem * item = reinterpret_cast<InternalItem*>(task->userData());
			if (!item) {
				item = new InternalItem();
			}
            item->index = j;
			item->task = task;
			item->task->setUserData(item);
            //m_objMap[item->task.get()] = item;
        }

    }
}

UploadTreeModel::InternalItem* UploadTreeModel::getInternalItem(const QModelIndex &index) {
	InternalItem * item = reinterpret_cast<InternalItem*>(index.internalPointer());
	return item;
}

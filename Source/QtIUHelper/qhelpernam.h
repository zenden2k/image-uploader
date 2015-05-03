#ifndef QHELPERNAM_H
#define QHELPERNAM_H

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QBuffer>

struct PostField
{
	QString name;
	QString value;
	bool isFile;
};

class QHelperNAM : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit QHelperNAM(QObject *parent = 0);
	 virtual QNetworkReply* createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );
	//bool parseMultiPartData(QList<PostField>* fields, QByteArray data);
signals:
	void urlLoad(QString url);
	void onPost(QString postUrl, QList<PostField>* fields);
	void onUpload(QString uploadUrl, QList<PostField>* fields);
private:
	QMap<QIODevice*, QBuffer*> m_DeviceBufferMap;
	bool parseMultiPartData(QList<PostField>* fields, QByteArray data, QString boundary);
	void parsePostData(QList<PostField>* fields, QByteArray data);
};


#endif // QHELPERNAM_H

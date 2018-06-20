#include "qhelpernam.h"
#include <QMessageBox>
#include <QFile>
QHelperNAM::QHelperNAM(QObject *parent) :
    QNetworkAccessManager(parent)
{
}

QNetworkReply* QHelperNAM::createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
{
	if(op == QNetworkAccessManager::PostOperation)
	{
		emit urlLoad(req.url().toString() + QString::number(outgoingData->bytesAvailable()));
		QBuffer *buf = new QBuffer;

		m_DeviceBufferMap[outgoingData] = buf;
		QByteArray* data =  new QByteArray();
		*data = outgoingData->readAll();

		QBuffer *dataDevice = new QBuffer(data, this);
		/*QFile f("g:\\data.txt");
		f.open(QIODevice::WriteOnly);
		f.write(*data);
		f.close();*/
		QString contentType = req.rawHeader("Content-Type");
		//QMessageBox::information(0, "Data", contentType);
		QRegExp rxlen("boundary=([\\-\\w\\d]+)");
		rxlen.setPatternSyntax(QRegExp::RegExp2);
		int pos = rxlen.indexIn(contentType);
		bool upload = false;
		bool hasFileFields = false;
		QList<PostField> fields;
		if (pos > -1) {
			 QString boundary = rxlen.cap(1);
			hasFileFields = parseMultiPartData(&fields, *data, boundary);
			if(hasFileFields)
				upload = true;

		}
		else if(contentType.contains("x-www-form-urlencoded"))
		{
			parsePostData(&fields, *data);
		}
		if(upload )
			emit onUpload(req.url().toString(), &fields);
		else
			emit onPost(req.url().toString(), &fields);
		outgoingData = dataDevice;
	}
	return QNetworkAccessManager::createRequest(op, req,outgoingData);
}

QString regexCap(QString text, QString regex)
{
	QRegExp rxlen(regex);
	rxlen.setPatternSyntax(QRegExp::RegExp2);
	int pos = rxlen.indexIn(text);
	if (pos > -1) {
		return rxlen.cap(1);
	}
	return QString();
}

bool QHelperNAM::parseMultiPartData(QList<PostField>* fields, QByteArray data, QString boundary)
{
	boundary = "--" + boundary;
	bool result = false;
	//QString strData = QString::fromUtf8(data.data());
	int pos = -1;
	for(;;)
	{
		pos = data.indexOf(boundary, pos+1);
		if(pos < 0 ) break;
		int start = pos + boundary.length();
		int endPos = data.indexOf(boundary, start);
		if(endPos < 0) break;
		QByteArray fieldData = data.mid(start, endPos - start);
		int dataStart = fieldData.indexOf("\r\n\r\n");
		if(dataStart < 0 )
			QMessageBox::information(0, "Data", "end not found");
		QString fieldHeader = QString::fromUtf8(fieldData.left(dataStart).data()).trimmed();
		QString fieldName = regexCap(fieldHeader, "name=\"(\\S+)\"");
		QString fieldValue;
		bool isFileField = false;
		if(fieldHeader.contains("filename=",Qt::CaseInsensitive))
		{
			isFileField = true;
		}
		if(!isFileField)
		{
			fieldValue = QString::fromUtf8(fieldData.mid(dataStart).trimmed());
		}
		else
		{
			int fieldSize = fieldData.size()-dataStart-6;
			if(fieldSize == 0) continue;
			//QMessageBox::information(0,"",QString("Field ' %1' size='%2'").arg(fieldName).arg(fieldSize));
		}
		if(isFileField)
			result = true ;
		PostField pf;
		pf.isFile = isFileField;
		pf.name = fieldName;
		pf.value= fieldValue;
		fields->push_back(pf);

 }
return result;
}

void QHelperNAM::parsePostData(QList<PostField>* fields, QByteArray data)
{
	QString str = QString::fromUtf8(data.data());
	//QMessageBox ::information(0,"lol",str);
	QStringList fieldsList = str.split('&');
	for(int i =0; i< fieldsList.count(); i++)
	{
		//QMessageBox ::information(0,"lol",fieldsList[i]);
		QStringList keyvalue = fieldsList[i].split('=');

		PostField field;
		if(keyvalue.count()>0)
			field.name = keyvalue[0];
		if(keyvalue.count()>1)
			field.value = keyvalue[1];
		fields->push_back(field);
	}
}


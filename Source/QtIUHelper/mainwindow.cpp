#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QWebFrame>
#include <QMessageBox>
#include "loginparamsform.h"
#include <QStackedLayout>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
   ui->setupUi(this);
    m_nam = new QHelperNAM(this);
	  QWebSettings::globalSettings ()  ->setAttribute(QWebSettings::PluginsEnabled,true);
	  int i;
	 connect(m_nam, SIGNAL(urlLoad(QString)), this, SLOT(namUrlLoad(QString)));
	 connect(m_nam, SIGNAL(onPost(QString,QList<PostField>*)), this, SLOT(onPost(QString,QList<PostField>*)));
	 connect(m_nam, SIGNAL(onUpload(QString,QList<PostField>*)), this, SLOT(onUpload(QString,QList<PostField>*)));
	 connect(ui->urlEdit, SIGNAL(returnPressed()), this, SLOT(on_goButton_clicked()));
	 ui->webView->page()->setNetworkAccessManager(m_nam);
	 connect(ui->regexpEdit, SIGNAL(textChanged(QString)), this, SLOT(updateCode()));
	connect(ui->assignVarsEdit, SIGNAL(textChanged(QString)), this, SLOT(updateCode()));
	  connect(ui->webView, SIGNAL(urlChanged(QUrl)), this, SLOT(onUrlChanged(QUrl)));
	  connect(ui->webView, SIGNAL(titleChanged(QString)), this, SLOT(setWindowTitle(QString)));
		connect(ui->webView->page(), SIGNAL(statusBarMessage(QString)), ui->statusLabel, SLOT(setText(QString)));
	  ui->listWidget->hide();
	  ui->verticalLayout->removeItem(ui->resultsLayout);
	  QWidget *w = new QWidget();
	  ui->resultsLayout->setParent(0);
	  w->setLayout(ui->resultsLayout);
			 ui->verticalLayout->addWidget(w);
}

void MainWindow::onUrlChanged(QUrl url)
{
	 ui->urlEdit->setText(url.toString());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_goButton_clicked()
{
	QUrl url=
			QUrl::fromUserInput(ui->urlEdit->text());
	 ui->webView->load(url);
}

 void  MainWindow::namUrlLoad(QString url)
 {
	 ui->listWidget->addItem(url);
 }

QString generateAction(QString Url, QString method, QList<PostField>& fields)
{
	QString postString;
	for(int i=0; i<fields.size(); i++)
	{
		QString actValue = fields[i].value;

		if(method == "upload"){
			if(fields[i].isFile)
				actValue = "%filename%";
			else if(fields[i].value.toInt() == 160)
				actValue = "$(THUMBWIDTH)";
		}

		postString+= fields[i].name + "=" + actValue;
		postString+=";";
	}
	if(method =="upload")
		return QString("\t\t<Action Type=\"upload\" Url=\"%1\" PostParams=\"%2\"%regexp%%assignvars%/>").arg(Url, postString);
	else
		return QString("\t\t<Action Type=\"post\" Url=\"%1\" PostParams=\"%2\"/>").arg(Url, postString);
}
void MainWindow::onPost(QString uploadUrl, QList<PostField>* fields)
{
	LoginParamsForm form(*fields, this);
	if(form.exec() != QDialog::Accepted) return;

	(*fields)[form.getLoginFieldIndex()].value="$(_LOGIN)";
	(*fields)[form.getPasswordFieldIndex()].value="$(_PASSWORD)";
	loginStr = "\r\n" + generateAction(uploadUrl, "post", *fields) + "\r\n";
	updateCode();
}

 void  MainWindow::onUpload(QString postUrl, QList<PostField>* fields)
 {
	QString res;
	QUrl url(postUrl);

	res += QString("<Server Name=\"%1\">\r\n").arg(url.host());
	res += "\t<Actions>\r\n";
	res += "%loginstr%";
	res +=generateAction(postUrl, "upload", *fields) +"\r\n";
	res += "\t</Actions>\r\n";
	res += "<Result ImageUrlTemplate=\"$(Image)\" ThumbUrlTemplate=\"$(Thumb)\"/>\r\n";
	res += "</Server>";
	generatedConfigTemplate = res;
	updateCode();
 }

 void MsgBox(QString msg)
 {
	 QMessageBox bx;
	 bx.setText(msg );
	 bx.setTextFormat(Qt::PlainText);
	 bx.exec();
 }

int CheckPattern(QString body, QString pattern, QString url)
{
	int pos = 0;
	int i=0;
	//MsgBox("ChecPattern"+ pattern+"\r\n\r\n"+body);
	for(;;)
	{
		 pos = body.indexOf(pattern, pos);
		if(pos<0) return -1;
		pos+=pattern.length();
	//	MsgBox( pattern+"\r\n"+"Bodymid="+body.mid(pos, url.length()));
		if(body.mid(pos, url.length()).startsWith(url))
			return i;
		i++;
		//pos+= pattern.length();
	}
	return i;
}

bool tryPosition(QString pageText, int pos, QString url,  QString& outPattern, int &outPatternIndex)
{
	//MsgBox("Position"+ QString::number(pos));
	QString before;
	for(int i=pos-1; i>0; i--)
	{
		if( pageText[i]=='[' || pageText[i]=='<' )
		{
			before = pageText.mid(i, pos-i);
			int patternIndex = CheckPattern(pageText, before, url);
			if(patternIndex>=0 && patternIndex<3)
			{
				outPattern = before;
				outPatternIndex = patternIndex;
				return true;
			}

		}
	}
	return false;
}
//int findBestPosition(QList<int> positions, QString pageText)
struct PatternItem
{
	QString leftMargin;
	QString rightMargin;
	int patternIndexInText;
	int startPos;
	int endPos;
	QString varName;
};

bool operator<(PatternItem p1, PatternItem p2)
{
	return p1.startPos < p2.endPos;
}

bool buildPattern(QString pageText, QString directUrl, PatternItem& result)
{
	QList<int> positions;
	int pos = 0;
	for(;;)
	{
		pos = pageText.indexOf(directUrl, pos);
		if(pos< 0) break;
		positions.push_back(pos);
		pos += directUrl.length();
	}
	if(positions.size() == 0) return false;

	QString pattern;
	QString rightmarginStr;
	QList<int> positionsArranged;
	// Rearrange positions
	for(int i=0; i<positions.size(); i++)
	{
		int pos = positions[i];
		if(pos == 0) continue;
		if(pageText[pos-1] == ']')
		{
			positions.removeAt(i);
			positionsArranged.push_back(pos);
		}
	}

	for(int i=0; i<positions.size(); i++)
	{
		int pos = positions[i];
		positionsArranged.push_back(pos);
	}
	int outPatIndex = -1;
	for(int i=0; i<positionsArranged.size(); i++)
	{
		int pos = positionsArranged[i];
		if(pos == 0) continue;

		if(tryPosition(pageText, pos, directUrl, pattern, outPatIndex))
		{
			int rightMargin = pos + directUrl.length();
			int rightMarginEnd =  pageText.length();
			for(int j=rightMargin; j< pageText.length(); j++)
			{
				if(QString("\"'[]<>").contains(pageText[j]))
				{
					rightMarginEnd = j;
					break;
				}
			}
			result.startPos = pos - pattern.length();
			result.endPos   =  rightMarginEnd;
			rightmarginStr = pageText.mid(rightMargin, rightMarginEnd-rightMargin+1);
			break;
		}
	}
	result.leftMargin = pattern;
	result.rightMargin = rightmarginStr;
	result.patternIndexInText = outPatIndex;
	//MsgBox("We select: "+pageText.mid(result.startPos, result.endPos - result.startPos)+"\r\nLeft margin:\r\n"+pattern+"\r\nIndex="+QString::number(outPatIndex)+"\r\nRight margin: "+rightmarginStr);
	return true;
}

int recalcPatternIndex(QString text, int startPos, QString pattern, int posweneed)
{
    //MsgBox(QString("%1 %2 %3 %4").arg(text).arg(startPos).arg(pattern).arg(posweneed));
	int i = 1;
	for(;;)
	{
		startPos = text.indexOf(pattern, startPos);
		if(startPos< 0)
			return -1;
		if(startPos == posweneed)
			return i;
		startPos += pattern.length();
		i++;
	}
	return -1;
}

QString escapeRegexChars(QString s)
{
	QString fobiddenChars = "[].-";
	for(int i=0; i< fobiddenChars.size(); i++)
	{
		s.replace(fobiddenChars[i], QString("\\%1").arg(fobiddenChars[i]));
	}
	return s;
}

QString EncodeXML(const QString& encodeMe)
{
	QString temp;

		  for (int index(0); index < encodeMe.size(); index++)
	{
	QChar character(encodeMe.at(index));

	switch (character.unicode())
	{
	case '&':
	temp += "&amp;"; break;

	case '\'':
	temp += "&apos;"; break;

	case '"':
	temp += "&quot;"; break;

	case '<':
	temp += "&lt;"; break;

	case '>':
	temp += "&gt;"; break;

	default:
	temp += character;
	break;
	}
	}

	return temp;
}

void MainWindow::on_analisePage_clicked()
{
	QString directUrl = ui->directUrlEdit->text();
	QString thumbUrl = ui->thumbUrlEdit->text();
	QString viewUrl = ui->viewUrlEdit->text();

	QString pageText =  ui->webView->page()->currentFrame()->toHtml();
	QList<PatternItem> patterns;
	if(!directUrl.isEmpty())
	{
		PatternItem it;
		it.varName = "Image";
		if(buildPattern(pageText,directUrl, it))
			patterns.push_back(it);

	}

	if(!thumbUrl.isEmpty())
	{
		PatternItem it;
		it.varName = "Thumb";
		if(buildPattern(pageText, thumbUrl, it))
			patterns.push_back(it);
	}

	if(!viewUrl.isEmpty())
	{
		PatternItem it;
		it.varName = "ViewUrl";
		if(buildPattern(pageText, viewUrl, it))
			patterns.push_back(it);
	}

	if(patterns.isEmpty())
		return;
	qSort(patterns);
	QString regexStr;
	int startPos = 0;
	int regexpMaskIndex = 0;
	QString assignVars;
	for(int i=0; i<patterns.size();  i++)
	{
		int count = recalcPatternIndex(pageText, startPos, patterns[i].leftMargin, patterns[i].startPos);
		if(count == -1) continue;
		startPos=patterns[i].endPos+1;
        //MsgBox(QString("Pattern:%1\r\nRecalculated: %2").arg(patterns[i].leftMargin).arg(count));
		for(int j=0; j< count; j++)
		{
			regexStr += escapeRegexChars(patterns[i].leftMargin);
			if(j!=count-1)
			{
				regexStr += "([\\s\\S]*?)";
				regexpMaskIndex++;
			}
		}
		regexStr += "(.*?)";
		assignVars += QString("%1:%2;").arg(patterns[i].varName).arg(regexpMaskIndex);
		regexpMaskIndex++;
		regexStr += escapeRegexChars(patterns[i].rightMargin);
		if(i != (patterns.size()-1))
		{
				regexStr += "([\\s\\S]*?)";
				regexpMaskIndex++;
		}
	}
	ui->regexpEdit->setText(regexStr);
	ui->assignVarsEdit->setText(assignVars);
	//MsgBox(regexStr + "\r\n" +assignVars);

}

void MainWindow::updateCode()
{
	QString regex = ui->regexpEdit->text();
	QString assignVars = ui->assignVarsEdit->text();
	QString code = generatedConfigTemplate;
	if(!regex.isEmpty())
		regex=" RegExp=\""+EncodeXML(regex)+"\"";
	if(!assignVars.isEmpty())
		assignVars=" AssignVars=\""+assignVars+"\"";

	code.replace("%regexp%",regex );
	code.replace("%assignvars%", assignVars);
	code.replace("%loginstr%", loginStr);
	ui->plainTextEdit->setPlainText(code);
}

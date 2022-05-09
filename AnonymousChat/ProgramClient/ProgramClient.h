#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProgramClient.h"

#include "DialogNewConnection.h"

#include "../../Utility/TCP.h"

#include <qevent.h>
#include <string>

#include <QTimer>
#include <QScrollBar>

using namespace std::chrono_literals;


class ProgramClient : public QMainWindow
{
	Q_OBJECT

public:
	ProgramClient(QWidget* parent = nullptr);

private:

	void NewConnection();

	void ReceiveMsg();

	
	bool eventFilter(QObject* object, QEvent* event);



	void SendMsg();


	Ui::ProgramClientClass ui;
	std::string serverAddress;
	std::string serverPort;
	std::string userName;

	TCP* tcp = nullptr;

	int currentPage = 1;
	int lineCount = 0;


private slots:

	void on_pushButtonSend_clicked();
	void on_lineEditMessage_returnPressed();

};

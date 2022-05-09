#include "ProgramClient.h"


ProgramClient::ProgramClient(QWidget* parent)
{
	ui.setupUi(this);

	NewConnection();

	QTimer* timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, &ProgramClient::ReceiveMsg);
	timer->start(100);
}

void ProgramClient::NewConnection()
{
	DialogNewConnection dialog;

	if (dialog.exec() != QDialog::DialogCode::Accepted)
		;

	serverAddress = dialog.serverAddress;
	serverPort = dialog.serverPort;
	userName = dialog.userName;

	if (serverAddress.empty())
		serverAddress = "127.0.0.1";

	if (serverPort.empty())
		serverPort = "9510";

	tcp = new TCP(serverPort, serverAddress);

	tcp->SendMsg("--username " + userName);
	tcp->WaitEvent();

	ui.statusBar->setText(QString::fromStdString(tcp->ReadMsg()));

	ui.lineEditMessage->installEventFilter(this);
}

void ProgramClient::ReceiveMsg()
{
	switch (tcp->WaitEvent(0))
	{
	case TCP::WaitEventType::MESSAGE:
	{
		std::string msg = tcp->ReadMsg();

		if (msg.substr(0, 8) == "[SERVER]")
			ui.textEditChat->setTextColor(Qt::blue);
		else
			ui.textEditChat->setTextColor(Qt::black);
		
		ui.textEditChat->append(QString::fromLocal8Bit(msg.c_str()));
		ui.textEditChat->verticalScrollBar()->setValue(ui.textEditChat->verticalScrollBar()->maximum());

		break;
	}

	case TCP::WaitEventType::DISCONNECT:

		break;
	}
}

bool ProgramClient::eventFilter(QObject* object, QEvent* event)
{
	if (object == ui.lineEditMessage) {
		QEvent::Type type = event->type();
		if (event->type() == QEvent::KeyPress) {

			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			qDebug() << "PRESS : " << keyEvent->key() << '\n';


			if (keyEvent->key() == Qt::Key::Key_Space) {
				ui.lineEditMessage->setText(ui.lineEditMessage->text() + ' ');
				return true;
			}
		}

		if (event->type() == QEvent::KeyRelease)
		{
			QKeyEvent* keyEvent = (QKeyEvent*)event;

			qDebug() << "RELEASE : " << keyEvent->key() << '\n';

			if (keyEvent->key() == Qt::Key::Key_Space) {
				return true;
			}

		}
	}
	return QMainWindow::eventFilter(object, event);
}

void ProgramClient::SendMsg()
{
	tcp->SendMsg(ui.lineEditMessage->text().toLocal8Bit().toStdString(), TCP::SendTo::ALL);

	ui.lineEditMessage->clear();
}


void ProgramClient::on_pushButtonSend_clicked()
{
	SendMsg();
}

void ProgramClient::on_lineEditMessage_returnPressed()
{
	SendMsg();
}
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
		return;

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
}

void ProgramClient::ReceiveMsg()
{
	switch (tcp->WaitEvent(0))
	{
	case TCP::WaitEventType::MESSAGE:
		ui.textEditChat->setText(ui.textEditChat->toPlainText() + QString::fromLocal8Bit(tcp->ReadMsg().c_str()) + "\n");
		ui.textEditChat->verticalScrollBar()->setValue(ui.textEditChat->verticalScrollBar()->maximum());

		break;

	case TCP::WaitEventType::DISCONNECT:

		break;
	}
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
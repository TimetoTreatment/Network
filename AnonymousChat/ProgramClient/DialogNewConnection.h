#pragma once

#include <QtWidgets/QDialog>
#include "ui_DialogNewConnection.h"

class DialogNewConnection : public QDialog
{
	Q_OBJECT

public:
	DialogNewConnection(QWidget* parent = Q_NULLPTR)
	{
		ui.setupUi(this);

		connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &DialogNewConnection::Accept);
		connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	}

	std::string serverAddress;
	std::string serverPort;
	std::string userName;


private:
	Ui::DialogNewConnection ui;

	void Accept()
	{
		serverAddress = ui.lineEdit1Address->text().toLocal8Bit().toStdString();
		serverPort = ui.lineEdit2Port->text().toLocal8Bit().toStdString();
		userName = ui.lineEdit3UserName->text().toLocal8Bit().toStdString();

		done(QDialog::Accepted);
	}

};

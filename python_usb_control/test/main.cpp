#include <iostream>
#include <csignal>
#include <ctime>

#include <QCoreApplication>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <qnamespace.h>

/**
*
*
*/

#define PORT 3000
QTcpServer *server = NULL;

void handleConnection(QTcpSocket *sock)
{
	printf("New incoming connection\n");
	printf("Closing connection\n");
	sock->close();
	delete sock;
}

void newConnection()
{
	while(server->hasPendingConnections())
		{
			QTcpSocket *connection = server->nextPendingConnection();
			handleConnection(connection);
		}
}

int main( int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	server = new QTcpServer();

	if(server->listen(QHostAddress::Any, PORT))
	{
		QObject::connect(server, &QTcpServer::newConnection, newConnection);
	}
	else {
		printf("Cannot listen \n");
		exit(-1);
	}
	printf(" Listening on port %d\n",PORT);
	while(1)
		{
		QCoreApplication::processEvents();
	}
	return 0;
}


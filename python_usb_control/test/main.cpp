#include <iostream>
#include <csignal>
#include <ctime>

#include <QCoreApplication>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <qnamespace.h>


extern void pseudoUsbServer();
extern void dsoInitUsb();
/**
*
*
*/


int main( int argc, char **argv)
{
	QCoreApplication app(argc, argv);
	
    pseudoUsbServer();
	dsoInitUsb();

	while(1)
	{
		QCoreApplication::processEvents();
	}

	return 0;
}


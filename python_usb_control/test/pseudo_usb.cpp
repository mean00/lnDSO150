


#include <iostream>
#include <csignal>
#include <ctime>

#include <QCoreApplication>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <qnamespace.h>

#include "include/lnUsbCDC.h"
#include "include/lnUsbStack.h"

/**
*
*
*/

#define PORT 3000
QTcpServer *server = NULL;

lnUsbCDC::lnUsbCDCEventsHandler *cdc_handler=NULL;
void *cdc_cookie=NULL;

lnUsbStack::lnUsbStackEventHandler *usb_handler=NULL;
void *usb_cookie=NULL;

/**
*/
extern void dsoInitUsb();
volatile bool alive=false;
QTcpSocket *current_connection=NULL;

void disc()
{
    printf("Disconnect!\n");
    alive=false;
}

void handleConnection(QTcpSocket *sock)
{
	printf("New incoming connection\n");
	QObject::connect(sock, &QTcpSocket::disconnected, disc);
    alive=true;
    current_connection=sock;
    cdc_handler(cdc_cookie, 0, lnUsbCDC::CDC_SESSION_START, 0);
    while(alive)
    {
            if( sock->waitForReadyRead(10))
            {
                cdc_handler(NULL,0, lnUsbCDC::CDC_DATA_AVAILABLE, 0);
            }
    }
    current_connection=NULL;
    printf("Closing connection\n");
    cdc_handler(cdc_cookie, 0, lnUsbCDC::CDC_SESSION_END, 0);
	sock->close();
	delete sock;
}

/**
*/

void newConnection()
{
	while(server->hasPendingConnections())
		{
			QTcpSocket *connection = server->nextPendingConnection();
			handleConnection(connection);
		}
}
/**
*/

int pseudoUsbServer( )
{
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
	
	return 0;
}

/**
*/

 int lnUsbCDC::read(uint8_t *buffer, int maxSize)
 {
    qint64 n=current_connection->bytesAvailable();
    if(n>maxSize)
        n=maxSize;
    int r= current_connection->read((char *)buffer,n);
    printf("Got %d \n",r);
    return r;
 }
 /**
*/

int lnUsbCDC::write(const uint8_t *buffer, int maxSize)
{
    return current_connection->write((const char *)buffer, maxSize);
}
/**
*/

void lnUsbCDC::flush()
{

}
/**
*/

void lnUsbCDC::setEventHandler(lnUsbCDCEventsHandler *h, void *cooki)
{
    cdc_handler=h;
    cdc_cookie=cooki;
}
/**
*/
void lnUsbCDC::clear_input_buffers()
{

}
/**
*/
lnUsbCDC::lnUsbCDC(int ev)
{

}

//----------------------------------
lnUsbStack::lnUsbStack()
{

}
void lnUsbStack::start()
{
    
}
void lnUsbStack::stop()
{

}
void lnUsbStack::setEventHandler(void *cookie, lnUsbStackEventHandler *ev)
{
    usb_handler=ev;
    usb_cookie=cookie;
}
void lnUsbStack::init(int nbDescriptorLine, const char **deviceDescriptor)
{

}
void lnUsbStack::setConfiguration(const uint8_t *hsConfiguration, const uint8_t *fsConfiguration,
    const tusb_desc_device_t *desc, const tusb_desc_device_qualifier_t *qual)
{
    
}

//-------------------------------------
 void lnUsbDFURT::addDFURTCb(lnDFUCb *fun)
 {

 }

//-----------------------------------------
   const char  **device_descriptor;
   tusb_desc_device_qualifier_t  desc_device_qualifier;
  tusb_desc_device_t desc_device;

  uint8_t  *desc_fs_configuration;
  uint8_t *desc_hs_configuration;
//------------------------------------------
void lnHardSystemReset()
{

}
// EOF
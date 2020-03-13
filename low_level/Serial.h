#ifndef _SERIAL_h
#define _SERIAL_h

#include <DynamixelAX12.h>

extern CommunicationServer Server;
extern OneWireMInterface SerialAX12;
extern OneWireMInterface SerialToF;
extern OneWireMInterface SerialExt;

#ifdef __cplusplus
#ifndef __INTELLISENSE__
extern "C"
{
#endif
#endif

void init_serial_ports();

#ifdef __cplusplus
#ifndef __INTELLISENSE__
}
#endif
#endif

#endif

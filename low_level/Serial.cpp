#include "Serial.h"
#include "Config.h"

CommunicationServer Server = CommunicationServer(SERIAL_HL);
OneWireMInterface SerialAX12 = OneWireMInterface(SERIAL_AX12);
OneWireMInterface SerialToF = OneWireMInterface(SERIAL_TOF);
OneWireMInterface SerialExt = OneWireMInterface(SERIAL_EXT);

void init_serial_ports()
{
    SERIAL_HL.begin(SERIAL_HL_BAUDRATE);
    SerialAX12.begin(SERIAL_AX12_BAUDRATE, SERIAL_AX12_TIMEOUT);
    SerialToF.begin(SERIAL_TOF_BAUDRATE, SERIAL_TOF_TIMEOUT);
    SerialExt.begin(SERIAL_EXT_BAUDRATE, SERIAL_EXT_TIMEOUT);
}

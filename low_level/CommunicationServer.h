#ifndef _COMMUNICATIONSERVER_h
#define _COMMUNICATIONSERVER_h

#include <Arduino.h>
#include <Printable.h>
#include <vector>
#include "Command.h"

/* Configurations diverses */
#define COMMAND_BUFFER_SIZE     8    // must be >= 2
#define OUTPUT_BUFFER_SIZE      255
#define MAX_RECEPTION_DURATION  500  // µs
#define ASYNC_TRACE_FILENAME    "ISR"


enum Channel
{
    ODOMETRY_AND_SENSORS    = 0x00,
    INFO                    = 0x01,
    ERROR                   = 0x02,
    TRACE                   = 0x03,
    SPY_ORDER               = 0x04,
    DIRECTION               = 0x05,
    AIM_TRAJECTORY          = 0x06,
    PID_SPEED               = 0x07,
    PID_TRANS               = 0x08,
    PID_TRAJECTORY          = 0x09,
    BLOCKING_MGR            = 0x0A,
    STOPPING_MGR            = 0x0B
};


class CommunicationServer
{
public:
    /* Constructeur */
    CommunicationServer();

    /* Envoie les messages de la file d'attente et lit les messages entrants */
    void communicate();

    /* Renvoie le nombre d'ordres présents dans la file d'attente */
    uint8_t available() const;

    /* Revoie la commande la plus ancienne de la file d'attente. */
    const Command &getLastCommand() const;

    /* Supprime la commande la plus ancienne de la file d'attente. */
    void discardLastCommand();

    /* Envoie la commande passée en argument, avec une trame standard */
    void sendAnswer(const Command &answer);

    /* Envoi de données spontanées avec une trame standard */
    void sendData(Channel channel, std::vector<uint8_t> const &data);

    /* Méthodes permettant l'envoi de données spontanées avec des trames
     * d'information */
    void print(uint32_t n) { print(INFO, n); }
    void print(int32_t n) { print(INFO, n); }
    void print(double d) { print(INFO, d); }
    void println(uint32_t n) { print(INFO, n, true); }
    void println(int32_t n) { print(INFO, n, true); }
    void println(double d) { print(INFO, d, true); }
    void println() { println(INFO); }

    void print_err(uint32_t n) { print(ERROR, n); }
    void print_err(int32_t n) { print(ERROR, n); }
    void print_err(double d) { print(ERROR, d); }
    void print_err(const Printable & obj) { print(ERROR, obj); }
    void println_err(uint32_t n) { print(ERROR, n, true); }
    void println_err(int32_t n) { print(ERROR, n, true); }
    void println_err(double d) { print(ERROR, d, true); }
    void println_err() { println(ERROR); }

    void print(const Printable & obj) { print(INFO, obj); }
    void println(const Printable & obj) { print(obj); println(); }
    void println(Channel channel, const Printable & obj) {
        print(channel, obj); println(channel);
    }

    void print(Channel channel, const Printable & obj);
    void printf(const char* format, ...);
    void printf_err(const char* format, ...);
    void printf(Channel channel, const char* format, ...);
private:
    void print(Channel channel, uint32_t u, bool newLine = false);
    void print(Channel channel, int32_t n, bool newLine = false);
    void print(Channel channel, double d, bool newLine = false);
    void print(Channel channel, const char* str, bool newLine = false);
    void println(Channel channel);

    /* Envoie les 4 octets d'entête d'une trame d'information */
    void writeInfoFrameHeader(Channel channel)
    {
        Serial.write(COMMAND_HEADER);
        Serial.write(COMMAND_BROADCAST);
        Serial.write((uint8_t)channel);
        Serial.write(0xFF);
    }

public:
    /* Envoie une trame d'information sur la canal TRACE permettant de
     * retrouver la ligne de code et le fichier ayant appelé la méthode */
    void trace(uint32_t line, const char* filename, uint32_t timestamp = 0);

    /* Même utilisation que trace() mais utilisable depuis une interruption (le
     * message sera envoyé plus tard, depuis la boucle principale) */
    void asynchronous_trace(uint32_t line);

private:
    /* Envoie la chaine de caractères contenue dans outputBuffer sous forme de
     * trame d'information */
    void printOutputBuffer(Channel channel);

    /* Envoi d'un vecteur d'octets */
    size_t sendVector(std::vector<uint8_t> const &vect) const;

    /* Envoi d'une chaine de caractères C (avec le caractère de fin de chaine)
     */
    size_t sendCString(const char* str) const;

    struct ExecTrace
    {
        uint32_t lineNb;
        uint32_t timestamp;
    };

    char outputBuffer[OUTPUT_BUFFER_SIZE];
    Command commandBuffer[COMMAND_BUFFER_SIZE];
    uint8_t cBufferHead;
    uint8_t cBufferTail;

    std::vector<ExecTrace> asyncTraceVect;
    std::vector<ExecTrace> asyncTraceVectBis;
    volatile bool bisTraceVectUsed;
};


extern CommunicationServer Server;


#endif

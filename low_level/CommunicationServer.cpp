#include "CommunicationServer.h"

using std::vector;

CommunicationServer::CommunicationServer(Stream &aStream) :
    stream(aStream)
{
    cBufferHead = 0;
    cBufferTail = 0;
    outputBuffer[0] = '\0';
    bisTraceVectUsed = false;
}

void CommunicationServer::communicate()
{
    /* Réception des messages */
    bool receivedAtLeastOneByte;
    uint32_t startReceptionTime = micros();

    do {
        receivedAtLeastOneByte = false;

        if (stream.available() > 0) {
            if (available() >= COMMAND_BUFFER_SIZE - 1) {
                printf_err("Command buffer is full\n");
                break;
            }

            receivedAtLeastOneByte = true;
            uint8_t newByte = stream.read();

            if (!commandBuffer[cBufferHead].addByte(newByte)) {
                printf_err("Drop the byte: %u\n", newByte);
            }

            if (commandBuffer[cBufferHead].isReady()) {
                cBufferHead++;
                if (cBufferHead == COMMAND_BUFFER_SIZE) {
                    cBufferHead = 0;
                }
            }
        }
    } while (micros() - startReceptionTime < MAX_RECEPTION_DURATION &&
        receivedAtLeastOneByte);

    /* Envoi des messages à envoi différé (issus des interruptions) */
    noInterrupts();
    bisTraceVectUsed = !bisTraceVectUsed;
    interrupts();
    if (bisTraceVectUsed) {
        for (size_t i = 0; i < asyncTraceVect.size(); i++) {
            trace(asyncTraceVect.at(i).lineNb, ASYNC_TRACE_FILENAME,
                asyncTraceVect.at(i).timestamp);
        }
        asyncTraceVect.clear();
    }
    else {
        for (size_t i = 0; i < asyncTraceVectBis.size(); i++) {
            trace(asyncTraceVectBis.at(i).lineNb, ASYNC_TRACE_FILENAME,
                asyncTraceVectBis.at(i).timestamp);
        }
        asyncTraceVectBis.clear();
    }
}

uint8_t CommunicationServer::available() const
{
    if (cBufferHead >= cBufferTail) {
        return cBufferHead - cBufferTail;
    }
    else {
        return COMMAND_BUFFER_SIZE + cBufferHead - cBufferTail;
    }
}

const Command &CommunicationServer::getLastCommand() const
{
    return commandBuffer[cBufferTail];
}

void CommunicationServer::discardLastCommand()
{
    if (available() > 0) {
        commandBuffer[cBufferTail].clear();
        cBufferTail++;
        if (cBufferTail == COMMAND_BUFFER_SIZE) {
            cBufferTail = 0;
        }
    }
}

void CommunicationServer::sendAnswer(const Command &answer)
{
    size_t n = 0;
    size_t normal_n = (size_t)answer.getLength() + 4;
    vector<uint8_t> frame;

    answer.getFrame(frame);
    n += stream.write(COMMAND_HEADER);
    n += sendVector(frame);

    if (n != normal_n) {
        printf_err("Answer not entierly sent (%u/%u)\n", n, normal_n);
    }
}

void CommunicationServer::sendData(Channel channel,
    vector<uint8_t> const &data)
{
    if (data.size() > COMMAND_MAX_LENGTH) {
        printf_err("Data too big to fit in a standard frame (%u/%u)\n",
            data.size(), COMMAND_MAX_LENGTH);
        return;
    }

    size_t n = 0;
    size_t normal_n = data.size() + 4;

    n += stream.write(COMMAND_HEADER);
    n += stream.write(COMMAND_BROADCAST);
    n += stream.write((uint8_t)channel);
    n += stream.write((uint8_t)data.size());
    n += sendVector(data);

    if (n != normal_n) {
        printf_err("Data not entierly sent (%u/%u)\n", n, normal_n);
    }
}

void CommunicationServer::print(Channel channel, const Printable & obj)
{
    writeInfoFrameHeader(channel);
    stream.print(obj);
    stream.print('\0');
}

void CommunicationServer::printf(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer, OUTPUT_BUFFER_SIZE, format, args);
    va_end(args);
    printOutputBuffer(INFO);
}

void CommunicationServer::printf_err(const char * format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer, OUTPUT_BUFFER_SIZE, format, args);
    va_end(args);
    printOutputBuffer(ERROR);
}

void CommunicationServer::printf(Channel channel, const char * format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer, OUTPUT_BUFFER_SIZE, format, args);
    va_end(args);
    printOutputBuffer(channel);
}

void CommunicationServer::print(Channel channel, uint32_t u, bool newLine)
{
    writeInfoFrameHeader(channel);
    if (newLine) { stream.print(u); }
    else { stream.println(u); }
    stream.print('\0');
}

void CommunicationServer::print(Channel channel, int32_t n, bool newLine)
{
    writeInfoFrameHeader(channel);
    if (newLine) { stream.print(n); }
    else { stream.println(n); }
    stream.print('\0');
}

void CommunicationServer::print(Channel channel, double d, bool newLine)
{
    writeInfoFrameHeader(channel);
    if (newLine) { stream.print(d); }
    else { stream.println(d); }
    stream.print('\0');
}

void CommunicationServer::print(Channel channel, const char* str, bool newLine)
{
    writeInfoFrameHeader(channel);
    if (newLine) { stream.print(str); }
    else { stream.println(str); }
    stream.print('\0');
}

void CommunicationServer::println(Channel channel)
{
    writeInfoFrameHeader(channel);
    stream.println();
    stream.print('\0');
}

void CommunicationServer::writeInfoFrameHeader(Channel channel)
{
    stream.write(COMMAND_HEADER);
    stream.write(COMMAND_BROADCAST);
    stream.write((uint8_t)channel);
    stream.write(0xFF);
}

void CommunicationServer::trace(uint32_t line, const char* filename,
    uint32_t timestamp)
{
    writeInfoFrameHeader(TRACE);
    if (timestamp == 0) { stream.print(micros()); }
    else { stream.print(timestamp); }
    stream.print('_');
    stream.print(line);
    stream.print('_');
    stream.print(filename);
    stream.print('\0');
}

void CommunicationServer::asynchronous_trace(uint32_t line)
{
    ExecTrace trace;
    trace.lineNb = line;
    trace.timestamp = micros();
    if (bisTraceVectUsed) {
        asyncTraceVectBis.push_back(trace);
    }
    else {
        asyncTraceVect.push_back(trace);
    }
}

void CommunicationServer::printOutputBuffer(Channel channel)
{
    writeInfoFrameHeader(channel);
    sendCString(outputBuffer);
    outputBuffer[0] = '\0';
}

size_t CommunicationServer::sendVector(vector<uint8_t> const &vect) const
{
    return stream.write(vect.data(), vect.size());
}

size_t CommunicationServer::sendCString(const char* str) const
{
    return stream.write(str) + stream.print('\0');
}

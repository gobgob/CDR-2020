#include "Command.h"

#define RECEPTION_TIMEOUT 100  // ms

using std::vector;

Command::Command()
{
    state = EMPTY;
    source = 0;
    id = 0;
    expectedLength = 0;
    lastByteTimestamp = 0;
}

Command::Command(uint8_t aSource, uint8_t aId, vector<uint8_t> const& aData)
{
    state = READY;
    source = aSource;
    id = aId;
    if (aData.size() <= COMMAND_MAX_LENGTH) {
        data = aData;
    }
}

Command::Command(uint8_t aSource, uint8_t aId)
{
    state = READY;
    source = aSource;
    id = aId;
}

bool Command::addByte(uint8_t newByte)
{
    uint32_t now = millis();
    if (state != EMPTY && now - lastByteTimestamp > RECEPTION_TIMEOUT) {
        clear();
    }
    lastByteTimestamp = now;

    switch (state)
    {
    case EMPTY:
        if (newByte == COMMAND_HEADER) {
            state = EXPECT_SOURCE;
        }
        else {
            return false;
        }
        break;
    case EXPECT_SOURCE:
        if (newByte < COMMAND_BROADCAST) {
            source = newByte;
            state = EXPECT_ID;
        }
        else {
            clear();
            return false;
        }
        break;
    case EXPECT_ID:
        if (newByte > COMMAND_MAX_ID) {
            clear();
            return false;
        }
        else {
            id = newByte;
            state = EXPECT_LENGTH;
        }
        break;
    case EXPECT_LENGTH:
        if (newByte > COMMAND_MAX_LENGTH) {
            clear();
            return false;
        }
        else {
            expectedLength = newByte;
            if (expectedLength == 0) {
                state = READY;
            }
            else {
                state = EXPECT_DATA;
                data.reserve(expectedLength);
            }
        }
        break;
    case EXPECT_DATA:
        data.push_back(newByte);
        if (data.size() == expectedLength) {
            state = READY;
        }
        break;
    case READY:
        return false;
    default:
        return false;
    }

    return true;
}

void Command::clear()
{
    state = EMPTY;
    source = 0;
    id = 0;
    expectedLength = 0;
    data.clear();
}

bool Command::isReady() const
{
    return state == READY;
}

uint8_t Command::getSource() const
{
    return source;
}

uint8_t Command::getId() const
{
    return id;
}

uint8_t Command::getLength() const
{
    return (uint8_t)data.size();
}

const vector<uint8_t>& Command::getData() const
{
    return data;
}

void Command::getFrame(std::vector<uint8_t>& frame) const
{
    frame.clear();
    frame.reserve(data.size() + 3);
    frame.push_back(source);
    frame.push_back(id);
    frame.push_back((uint8_t)data.size());
    for (size_t i = 0; i < data.size(); i++) {
        frame.push_back(data.at(i));
    }
}

size_t Command::printTo(Print& p) const
{
    size_t n = 0;
    n += p.print("STATE: ");
    switch (state)
    {
    case EMPTY:
        n += p.print("EMPTY");
        break;
    case EXPECT_SOURCE:
        n += p.print("EXPECT_SOURCE");
        break;
    case EXPECT_ID:
        n += p.print("EXPECT_ID");
        break;
    case EXPECT_LENGTH:
        n += p.print("EXPECT_LENGTH");
        break;
    case EXPECT_DATA:
        n += p.print("EXPECT_DATA");
        break;
    case READY:
        n += p.print("READY");
        break;
    default:
        break;
    }
    n += p.print("S:");
    n += p.print(source);
    n += p.print(" ID:");
    n += p.print(id, HEX);
    if (data.size() > 0) {
        n += p.print(" Data: ");
        for (size_t i = 0; i < data.size(); i++) {
            n += p.print(data.at(i), HEX);
            n += p.print(" ");
        }
    }
    return n;
}

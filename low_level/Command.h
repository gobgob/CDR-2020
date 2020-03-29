#pragma once

#include <Arduino.h>
#include <vector>
#include <Printable.h>

#define COMMAND_HEADER      0xFF
#define COMMAND_BROADCAST   0xFE
#define COMMAND_MAX_ID      0xFE
#define COMMAND_MAX_LENGTH  0xFE


class Command : public Printable
{
public:
    Command();
    Command(uint8_t aSource, uint8_t aId, std::vector<uint8_t> const& aData);
    Command(uint8_t aSource, uint8_t aId);

    /* Ajoute un octet à la commande, renvoie vrai si l'octet est valide */
    bool addByte(uint8_t newByte);

    void clear();
    bool isReady() const;
    uint8_t getSource() const;
    uint8_t getId() const;
    uint8_t getLength() const;
    const std::vector<uint8_t>& getData() const;
    void getFrame(std::vector<uint8_t> &frame) const;
    size_t printTo(Print& p) const;

private:
    enum State {
        EMPTY,
        EXPECT_SOURCE,
        EXPECT_ID,
        EXPECT_LENGTH,
        EXPECT_DATA,
        READY,
    };

    State state;
    uint8_t source; // le numéro du client d'où provient cette commande
    uint8_t id; // l'ID de l'ordre de la commande
    uint8_t expectedLength;
    std::vector<uint8_t> data;
};

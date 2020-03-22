#ifndef _ORDERMGR_h
#define _ORDERMGR_h

#include "Serial.h"
#include "OrderImmediate.h"
#include "OrderLong.h"
#include "Command.h"

#define EXEC_STACK_SIZE             16
#define NB_ORDER                    256
#define LONG_ORDER_START_ID         32
#define IMMEDIATE_ORDER_START_ID    128
#define NB_LONG_ORDER (IMMEDIATE_ORDER_START_ID - LONG_ORDER_START_ID)
#define NB_IMMEDIATE_ORDER (NB_ORDER - IMMEDIATE_ORDER_START_ID)


class OrderMgr
{
public:
    OrderMgr();
    void execute();

private:
    /* Implémenté dans API.cpp */
    void initImmediateOrderList();
    void initLongOrderList();

    void handleNewCommand(const Command &command);
    void executeStackedOrders();
    int addOrderToStack(uint8_t index, const Command &command);

    class OrderMemory
    {
    public:
        OrderMemory()
        {
            running = false;
            longOrderIndex = 0;
            commandId = 0;
            commandSource = 0;
        }

        void saveOrder(uint8_t index, Command const &command)
        {
            longOrderIndex = index;
            commandId = command.getId();
            commandSource = command.getSource();
            running = true;
        }

        void deleteOrder() { running = false; }
        uint8_t getIndex() const { return longOrderIndex; }
        uint8_t getId() const { return commandId; }
        uint8_t getSource() const { return commandSource; }
        bool isRunning() const { return running; }

    private:
        uint8_t longOrderIndex;
        uint8_t commandId;
        uint8_t commandSource;
        bool running;
    };

    OrderMemory orderStack[EXEC_STACK_SIZE];
    OrderLong* longOrderList[NB_LONG_ORDER];
    OrderImmediate* immediateOrderList[NB_IMMEDIATE_ORDER];
};


#endif

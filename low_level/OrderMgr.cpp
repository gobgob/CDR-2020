#include "OrderMgr.h"

using std::vector;

OrderMgr::OrderMgr()
{
    for (size_t i = 0; i < NB_LONG_ORDER; i++) {
        longOrderList[i] = (OrderLong*)NULL;
    }
    for (size_t i = 0; i < NB_IMMEDIATE_ORDER; i++) {
        immediateOrderList[i] = (OrderImmediate*)NULL;
    }

    initImmediateOrderList();
    initLongOrderList();
}

void OrderMgr::execute()
{
    Server.communicate();
    while (Server.available()) {
        handleNewCommand(Server.getLastCommand());
        Server.discardLastCommand();
    }
    executeStackedOrders();
}

void OrderMgr::handleNewCommand(const Command &command)
{
    uint8_t id = command.getId();
    const vector<uint8_t>& input = command.getData();

    if (id >= IMMEDIATE_ORDER_START_ID) {
        uint8_t index = id - IMMEDIATE_ORDER_START_ID;

        if (index < NB_IMMEDIATE_ORDER && immediateOrderList[index] != NULL) {
            vector<uint8_t> output;
            immediateOrderList[index]->execute(input, output);
            if (output.size() > 0) {
                Command answer(command.getSource(), id, output);
                Server.sendAnswer(answer);
            }
        }
        else {
            Server.printf_err("Unknown immediate order: %u\n", index);
        }
    }
    else if (id >= LONG_ORDER_START_ID) {
        uint8_t index = id - LONG_ORDER_START_ID;

        if (index < NB_LONG_ORDER && longOrderList[index] != NULL) {
            if (addOrderToStack(index, command) == 0) {
                longOrderList[index]->launch(input);
            }
            else {
                Server.printf_err("Too many long orders already running\n");
            }
        }
        else {
            Server.printf_err("Unknown long order: %u\n", index);
        }
    }
    else {
        Server.printf_err("Invalid command id\n");
        Server.print_err(command);
    }
}

void OrderMgr::executeStackedOrders()
{
    for (size_t i = 0; i < EXEC_STACK_SIZE; i++) {
        if (orderStack[i].isRunning()) {
            uint8_t index = orderStack[i].getIndex();
            longOrderList[index]->onExecute();
            if (longOrderList[index]->isFinished()) {
                vector<uint8_t> output_data;
                longOrderList[index]->terminate(output_data);
                Command answer(orderStack[i].getSource(),
                    orderStack[i].getId(), output_data);
                Server.sendAnswer(answer);
                orderStack[i].deleteOrder();
            }
        }
    }
}

int OrderMgr::addOrderToStack(uint8_t index, const Command &command)
{
    for (size_t i = 0; i < EXEC_STACK_SIZE; i++) {
        if (!orderStack[i].isRunning()) {
            orderStack[i].saveOrder(index, command);
            return 0;
        }
    }
    return -1;
}

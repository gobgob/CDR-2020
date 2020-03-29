#pragma once

#define VARIABLE_INPUT_SIZE ((std::size_t)(-1))

#define ORDER_IMMEDIATE(name, input_size)                                     \
class name : public OrderImmediate, public Singleton<name> {                  \
public:                                                                       \
    name() { inputSize = (input_size); }                                      \
    virtual void _execute(const std::vector<uint8_t> &input,                  \
        std::vector<uint8_t> &output);                                        \
}

#define ORDER_IMMEDIATE_EXECUTE(name) \
void name::_execute(const std::vector<uint8_t> &input, \
std::vector<uint8_t> &output)

#define ORDER_LONG(name, input_size, private_data)                            \
class name : public OrderLong, public Singleton<name> {                       \
public:                                                                       \
    name() { inputSize = (input_size); }                                      \
    void _launch(const std::vector<uint8_t>& input);                          \
    void onExecute();                                                         \
    void terminate(std::vector<uint8_t>& output);                             \
private:                                                                      \
    private_data                                                              \
}

#define ORDER_LONG_LAUNCH(name) \
void name::_launch(const std::vector<uint8_t> &input)

#define ORDER_LONG_EXECUTE(name) \
void name::onExecute()

#define ORDER_LONG_TERMINATE(name) \
void name::terminate(std::vector<uint8_t> &output)

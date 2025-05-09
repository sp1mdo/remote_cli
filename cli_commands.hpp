#ifndef CLI_COMMANDS
#define CLI_COMMANDS

#include <string>
#include <cstdint>

void show_settings(void);
void show_pid(void);
void show_oil(void);
void show_temperature(void);
void show_relay_functions();
void show_input_functions();
void system_info(void);
void test_flow_value(uint16_t val);
void show_hot_water(void);
void test_equithermal_curve(int16_t ambient_temp);
void callback(int id, const std::string &str);
void holding_registers_show_all(void);
std::string getRelayModeStr(uint16_t mask);
std::vector<std::string> tokenize(const std::string &input);
#endif // CLI_COMMANDS
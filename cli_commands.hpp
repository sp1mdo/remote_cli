#ifndef CLI_COMMANDS
#define CLI_COMMANDS

#include <string>
#include <cstdint>
#include <vector>

extern std::vector<std::string> monitor_names;
extern std::vector<uint16_t> monitor_registers;

void show_settings(void);
void restore_default_settings(void);
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
const char *operationToString(uint16_t num);
std::string getHotWaterModeStr(uint16_t mode);
std::string getRelayModeStr(uint16_t mask);
std::vector<std::string> tokenize(const std::string &input);
void set_monitor(const std::string &str);
void show_monitor(void);
std::string to_string_with_precision(float value, int precision);
int8_t getInputRegScaleFactor(uint16_t reg);
#endif // CLI_COMMANDS
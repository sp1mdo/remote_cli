#ifndef CLI_COMMANDS
#define CLI_COMMANDS

#include <string>
#include <cstdint>
#include <vector>

extern std::vector<std::string> monitor_names;
extern std::vector<uint16_t> monitor_registers;

void show_settings(void);
void show_control(void);
void restore_default_settings(void);
void show_pid(void);
void show_oil(void);
void show_temperature(void);
void show_relay_functions();
void show_input_functions();
void system_info(void);
void test_flow_value(int16_t val);
void show_hot_water(void);
void test_equithermal_curve(int16_t ambient_temp);
void callback(int id, const std::string &str);
void holding_registers_show_all(void);
const char *operationToString(uint16_t num);
const char *getHotWaterModeStr(uint16_t mode);
std::string_view controlToSv(uint16_t val);
std::string getRelayModeStr(uint16_t mask);
std::vector<std::string> tokenize(const std::string &input);
void set_monitor(const std::string &str);
void show_monitor(void);
void show_defrost(void);
std::string to_string_with_precision(float value, int precision);
int8_t getInputRegScaleFactor(uint16_t reg);
float curve(float t4, float A, float B);
extern size_t getHash(const std::string &unique_id_str, size_t depth);
std::string get_unique_id_string();
#endif // CLI_COMMANDS
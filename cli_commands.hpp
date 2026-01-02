#ifndef CLI_COMMANDS
#define CLI_COMMANDS

#include <string>
#include <cstdint>
#include <vector>

void show_settings(void);
void show_control(void);
void registers_restore_default(void);
void show_pid(void);
void show_oil(void);
void show_temperature(void);
void show_relay_functions();
void show_input_functions();
void system_info(void);
void test_flow_value(int16_t val);
void show_dhw(void);
void show_bivalent(void);
void test_equithermal_curve(int16_t ambient_temp);
void callback(int id, const std::string &str);
void holding_registers_show_all(void);
const char *operationToString(uint16_t num);
const char *getDHWModeStr(uint16_t mode);
std::string_view controlToSv(uint16_t val);
std::string getRelayModeStr(uint16_t mask);
std::string getInputModeStr(uint16_t mask);
std::vector<std::string> tokenize(const std::string &input);
void monitor_add(const std::string &str);
void monitor_remove(const std::string &str);
void monitor_clear(void);
void monitor_show(void);
void show_defrost(void);
std::string to_string_with_precision(float value, int precision);
int8_t getInputRegScaleFactor(uint16_t reg);
float curve(float t4, float A, float B);
extern size_t getHash(const std::string &unique_id_str, size_t depth);
void calculate_curve(const std::string &str);
void calculate_curve(float x1, float y1, float x2, float y2);
void set_relay_polarity(uint8_t relay_no, const std::string &str);
std::string get_unique_id_string();
void show_softstart(void);

extern bool g_preheat_request;
#endif // CLI_COMMANDS
#include <memory>

class Config {
public:
    // TODO add configuration variables here
};

extern std::unique_ptr<Config> g_config;

void config_load(int argc, char** argv);
void config_save();

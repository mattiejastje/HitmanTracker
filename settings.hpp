#include <memory>

class Settings {
   public:
    // TODO add configuration variables here
};

extern std::unique_ptr<Settings> g_settings;

void settings_load(int argc, char** argv);
void settings_save();

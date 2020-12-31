#ifndef UNI_CONFIG_H
#define UNI_CONFIG_H

typedef struct {
  int mode;
  char filename[40];
} Config;

class UniConfig
{

  public:
    UniConfig();
    bool readConfig(Config *);
    bool writeConfig(Config *config);
  private:
    Config _config;
};
#endif

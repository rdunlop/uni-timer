#ifndef UNI_CONFIG_H
#define UNI_CONFIG_H

// NOTE: Adjustments to this structure MUST ALSO
// be reflected in the readConfig and writeConfig methods
// or else the config won't persist/parse properly
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
    bool prefix(const char *str, const char *prefix);
    char *value(const char *str, const char *prefix);
};
#endif

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
    bool fileExists();
    char *filename();
    void setFilename(char *filename);
    int mode();
    void setMode(int mode);
  private:
    bool readConfig();
    bool writeConfig();
    bool prefix(const char *str, const char *prefix);
    char *value(const char *str, const char *prefix);
    Config _config;
};
#endif

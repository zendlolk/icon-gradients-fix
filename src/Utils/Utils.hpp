#include "Includes.hpp"

class Utils {

public:

    static SimplePlayer* createIcon(IconType, bool = false);

    static CCMenuItemToggler* createTypeToggle(bool, CCPoint, CCObject*, SEL_MenuHandler);
    
    static ccColor3B getPlayerColor(ColorType, bool);

    static int getIconID(IconType, bool = false);
    static bool isGradientSaved(GradientConfig);
    static bool isSettingEnabled(int);

    static std::string getTypeID(IconType);
    static std::string getTypeID(SpriteType);

    static IconType getIconType(SimplePlayer*);
    static std::vector<GradientConfig> getSavedGradients();

    static GradientConfig configFromObject(const matjson::Value&);
    static GradientConfig getSavedConfig(IconType, ColorType, bool = false);
    static GradientConfig getDefaultConfig(ColorType, bool);
    static GradientConfig getDefaultBarConfig();
    static GradientConfig getSavedBarConfig();
    static void saveBarConfig(GradientConfig);

    static GradientConfig getDefaultWaveConfig();
    static GradientConfig getSavedWaveConfig();
    static void saveWaveConfig(GradientConfig);
    static std::vector<WaveBand> computeWaveBands(const GradientConfig&);
    static const std::vector<WaveBand>& getWaveBands();
    static void invalidateWaveBands();
    static matjson::Value getSaveObject(GradientConfig);

    static Gradient getGradient(IconType, bool);

    static void removeSavedGradient(GradientConfig);
    static void saveConfig(GradientConfig, const std::string&, const std::string&);
    static void setIconColors(SimplePlayer*, ColorType, bool, bool = false);
    
    static void applyGradient(SimplePlayer*, Gradient, bool, bool, int);
    static void applyGradient(SimplePlayer*, GradientConfig, ColorType, bool, bool, int);
    static void applyGradient(CCSprite*, GradientConfig, IconType, ColorType, int, bool, bool, bool, int, bool = false, const CCRect* = nullptr);
    static void applyBarGradient(CCSprite*, GradientConfig, bool, const CCRect* = nullptr);
    
    static CCGLProgram* createShader(const std::string&, bool, bool, bool);
    
    static void patchBatchNode(CCSpriteBatchNode*);
    static void hideSprite(CCSprite*);

};
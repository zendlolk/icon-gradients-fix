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

    static std::string getWaveID(bool);
    static GradientConfig getDefaultWaveConfig(bool);
    static GradientConfig getSavedWaveConfig(bool);
    static void saveWaveConfig(GradientConfig, bool);
    static bool isGlowLinked();
    static void setGlowLinked(bool);
    static std::string getGlowID(GlowTarget, bool);
    static GlowSettings getGlow(GlowTarget, bool);
    static void saveGlow(GlowTarget, GlowSettings, bool);
    static GlowSettings getWaveGlow(bool);

    static CCSprite* updateSpriteGlow(CCSprite*, GradientConfig, IconType, int, bool, bool, int, const GlowSettings&);
    static CCSprite* updateBarGlow(CCSprite*, const CCRect&, GradientConfig, bool, const GlowSettings&);
    static void setBarGlowFill(CCSprite*, float);
    static float getWaveGlowReach(const GlowSettings&);
    static std::vector<WaveNode> computeWaveProfile(const GradientConfig&, const GlowSettings&);
    static const std::vector<WaveNode>& getWaveProfile(bool);
    static void appendWaveFan(const std::vector<WaveNode>&, const CCPoint&, const CCPoint&, const CCPoint&, float, std::vector<ccV2F_C4B_T2F>&);
    static void appendWaveSegment(const std::vector<WaveNode>&, const WaveEnd&, const WaveEnd&, float, std::vector<ccV2F_C4B_T2F>&);
    static void invalidateWaveBands();

    static void setDrawNodeVertices(CCDrawNode*, const std::vector<ccV2F_C4B_T2F>&);
    static matjson::Value getSaveObject(GradientConfig);

    static Gradient getGradient(IconType, bool);

    static void removeSavedGradient(GradientConfig);
    static void saveConfig(GradientConfig, const std::string&, const std::string&);
    static void setIconColors(SimplePlayer*, ColorType, bool, bool = false);
    
    static void applyGradient(SimplePlayer*, Gradient, bool, bool, int);
    static void applyGradient(SimplePlayer*, GradientConfig, ColorType, bool, bool, int);
    static void applyGradient(CCSprite*, GradientConfig, IconType, ColorType, int, bool, bool, bool, int, bool = false, const CCRect* = nullptr, bool = false, bool = false);
    static void applyBarGradient(CCSprite*, GradientConfig, bool, const CCRect* = nullptr);
    
    static CCGLProgram* createShader(const std::string&, bool, bool, bool, bool = false);
    
    static void patchBatchNode(CCSpriteBatchNode*);
    static void hideSprite(CCSprite*);

};
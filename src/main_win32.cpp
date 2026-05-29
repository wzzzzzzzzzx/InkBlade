#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <windowsx.h>
#include <wincodec.h>
#include <algorithm>
#include <array>
#include <cmath>
#include <random>
#include <string>
#include <vector>

namespace {

constexpr int W = 1280;
constexpr int H = 720;
constexpr float BaseDamage = 100.f;
constexpr float ArenaLeft = 88.f;
constexpr float ArenaRight = 1192.f;
constexpr float ArenaTop = 530.f;
constexpr float ArenaBottom = 650.f;
constexpr float QuickAttackThreshold = 0.18f;

struct Vec {
    float x = 0.f;
    float y = 0.f;
};

float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

float absf(float v) {
    return v < 0.f ? -v : v;
}

float len(Vec v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

Vec norm(Vec v) {
    const float l = len(v);
    return l < 0.01f ? Vec{} : Vec{v.x / l, v.y / l};
}

struct Character {
    const wchar_t* name;
    const wchar_t* title;
    float hp;
    float attack;
    float speed;
    float staminaRegen;
    float hitEnergy;
    float takenEnergy;
    COLORREF color;
    const wchar_t* passive;
    const wchar_t* skill;
    const wchar_t* ult;
};

struct Weapon {
    const wchar_t* name;
    const wchar_t* style;
    std::vector<float> combo;
    std::array<float, 3> charge;
    float parryWindow;
    float counter;
};

const std::array<Character, 3> Characters = {{
    {L"凌霜", L"输出型剑客", 900.f, 1.15f, 260.f, 15.f, 1.25f, 1.f, RGB(115, 205, 255), L"被动：所有伤害 +15%", L"F：寒冰突刺，冲刺并减速", L"V：冰华乱舞，霸体连斩"},
    {L"墨痕", L"控制型书生", 1000.f, 0.9f, 280.f, 15.f, 1.f, 1.f, RGB(165, 105, 230), L"被动：技能冷却 -20%", L"F：墨缚印，短暂定身", L"V：天罗墨网，大范围禁闪"},
    {L"素心", L"回复型医者", 1100.f, 0.85f, 235.f, 22.f, 1.f, 1.5f, RGB(255, 145, 195), L"被动：受伤后回复本次伤害 5%", L"F：回春术，回复 20% 生命", L"V：济世莲华，持续回复并减伤"}
}};

const std::array<Weapon, 3> Weapons = {{
    {L"长剑", L"均衡型", {1.f, 1.f, 1.5f}, {1.5f, 2.f, 3.f}, 0.17f, 1.2f},
    {L"大刀", L"重型", {1.3f, 2.f}, {2.f, 2.8f, 4.f}, 0.13f, 2.f},
    {L"双刃", L"速攻型", {0.6f, 0.6f, 0.8f, 0.8f, 1.2f}, {1.5f, 2.2f, 2.8f}, 0.2f, 0.8f}
}};

enum class Screen { Main, Character, Weapon, Difficulty, Practice, Battle, Result };
enum class Action { Idle, Normal, Charging, ChargeRelease, Parry, Counter, Dodge, Skill, Ultimate, Hit, Down };
enum class Difficulty { Easy, Normal, Hard };
enum class VisualLabAction { Ultimate, ChargeRelease, Hit, Skill, Normal, Charging, Jump, Parry, Dodge };
enum class ActionSpriteSlot {
    Idle,
    Run1,
    Run2,
    Jump,
    Dodge,
    Parry,
    Hit,
    Normal,
    Charging,
    ChargeRelease,
    Skill,
    Ultimate,
    Count
};

struct ActionSpriteLayout {
    int width;
    int height;
    int xOffset;
    int yOffset;
    bool mirrorFromSource;
};

const std::array<const wchar_t*, 3> CharacterSpriteFolder = {{
    L"characters\\lingshuang\\sprites\\body\\",
    L"characters\\mohen\\sprites\\body\\",
    L"characters\\suxin\\sprites\\body\\"
}};

const std::array<const wchar_t*, 3> CharacterVfxFolder = {{
    L"characters\\lingshuang\\sprites\\vfx\\",
    L"characters\\mohen\\sprites\\vfx\\",
    L"characters\\suxin\\sprites\\vfx\\"
}};

const std::array<const wchar_t*, 3> WeaponModelFolder = {{
    L"weapons\\longsword\\sprites\\model\\",
    L"weapons\\broadsword\\sprites\\model\\",
    L"weapons\\dualblades\\sprites\\model\\"
}};

struct Input {
    bool attackPress = false;
    bool attackRelease = false;
    bool attackHeld = false;
    bool parry = false;
    bool dodge = false;
    bool skill = false;
    bool ultimate = false;
    bool jump = false;
    Vec move;
};

struct Particle {
    Vec p;
    Vec v;
    COLORREF color;
    float life;
    float maxLife;
    float r;
};

enum class EffectType {
    Slash,
    ChargeLoop,
    ChargeRelease,
    ParryGuard,
    ParrySuccess,
    ParryBreak,
    DodgeTrail,
    HitBurst,
    SkillAura,
    UltimateAura
};

struct Effect {
    EffectType type;
    Vec p;
    COLORREF color;
    float life;
    float maxLife;
    float radius;
    int facing;
    int level;
    int role;
};

struct FloatingText {
    Vec p;
    Vec v;
    std::wstring text;
    COLORREF color;
    float life;
    float maxLife;
    int size;
};

struct Fighter {
    Character c = Characters[0];
    Weapon w = Weapons[0];
    Vec p;
    int facing = 1;
    int lastMoveFacing = 1;
    float hp = 1.f;
    float stamina = 100.f;
    float energy = 0.f;
    float cooldown = 0.f;
    float actionTimer = 0.f;
    float hitTimer = 0.f;
    float chargeTimer = 0.f;
    float comboTimer = 0.f;
    float dodgeLock = 0.f;
    float bindTimer = 0.f;
    float slowTimer = 0.f;
    float lotusTimer = 0.f;
    float flashTimer = 0.f;
    float z = 0.f;
    float vz = 0.f;
    int comboIndex = 0;
    bool airborne = false;
    Action action = Action::Idle;
    float damageDone = 0.f;
    int parries = 0;
    int maxCombo = 0;
    int chargeHits = 0;
    int dodges = 0;
};

struct BitmapImage {
    HBITMAP bitmap = nullptr;
    int width = 0;
    int height = 0;

    void destroy() {
        if (bitmap) {
            DeleteObject(bitmap);
            bitmap = nullptr;
        }
        width = 0;
        height = 0;
    }
};

class Game {
public:
    HWND hwnd = nullptr;

    ~Game() {
        for (auto& profile : characterSprites) {
            for (auto& sprite : profile) {
                sprite.destroy();
            }
        }
        for (auto& image : characterSelectBackgrounds) {
            image.destroy();
        }
        for (auto& image : weaponSelectBackgrounds) {
            image.destroy();
        }
        for (auto& image : weaponModelSprites) {
            image.destroy();
        }
        menuBackground.destroy();
        battleBackground.destroy();
        battleGround.destroy();
        if (wic) {
            wic->Release();
            wic = nullptr;
        }
        if (comInitialized) {
            CoUninitialize();
        }
    }

    void init(HWND window) {
        hwnd = window;
        initGraphics();
        visualLabMode = commandLineHas(L"--visual-lab");
        visualLabCharacter = commandLineInt(L"--visual-lab-character", 0, 0, 2);
        if (visualLabMode) {
            startVisualLab();
        } else {
            resetBattle();
        }
    }

    void onKey(WPARAM key) {
        if (screen == Screen::Battle) {
            if (visualLabMode) {
                if (key == '1') {
                    triggerVisualLabAction(VisualLabAction::Ultimate);
                    return;
                }
                if (key == '2') {
                    triggerVisualLabAction(VisualLabAction::ChargeRelease);
                    return;
                }
                if (key == '3') {
                    triggerVisualLabAction(VisualLabAction::Hit);
                    return;
                }
                if (key == '4') {
                    triggerVisualLabAction(VisualLabAction::Skill);
                    return;
                }
                if (key == '5') {
                    triggerVisualLabAction(VisualLabAction::Normal);
                    return;
                }
                if (key == '6') {
                    triggerVisualLabAction(VisualLabAction::Charging);
                    return;
                }
                if (key == '7') {
                    triggerVisualLabAction(VisualLabAction::Jump);
                    return;
                }
                if (key == '8') {
                    triggerVisualLabAction(VisualLabAction::Parry);
                    return;
                }
                if (key == '9') {
                    triggerVisualLabAction(VisualLabAction::Dodge);
                    return;
                }
            }
            if (key == VK_ESCAPE) {
                screen = Screen::Main;
                menu = 0;
            }
            return;
        }
        if (screen == Screen::Result) {
            if (key == VK_RETURN) {
                resetBattle();
                screen = Screen::Battle;
            }
            if (key == VK_ESCAPE) {
                screen = Screen::Main;
                menu = 0;
            }
            return;
        }

        const int count = optionCount();
        if (key == VK_HOME) {
            menu = 0;
            sync();
        }
        if (key == VK_END) {
            menu = count - 1;
            sync();
        }
        if (key == VK_UP || key == VK_LEFT || key == 'W' || key == 'A') {
            menu = (menu + count - 1) % count;
            sync();
        }
        if (key == VK_DOWN || key == VK_RIGHT || key == 'S' || key == 'D') {
            menu = (menu + 1) % count;
            sync();
        }
        if (key == VK_RETURN) {
            confirm();
        }
        if (key == VK_ESCAPE) {
            back();
        }
    }

    void onMouseMove(int x, int y) {
        if (screen == Screen::Battle || screen == Screen::Result) return;
        const int hit = menuHit(x, y);
        if (hit >= 0) {
            menu = hit;
            sync();
        }
    }

    void onMouse(bool down, int x, int y) {
        if (screen == Screen::Battle) {
            mouseDown = down;
            return;
        }

        mouseDown = false;
        lastMouse = false;
        if (screen == Screen::Result) {
            if (!down) {
                if (inside(x, y, 420, 520, 200, 52)) {
                    resetBattle();
                    screen = Screen::Battle;
                } else if (inside(x, y, 660, 520, 200, 52)) {
                    screen = Screen::Main;
                    menu = 0;
                }
            }
            return;
        }

        const int hit = menuHit(x, y);
        if (down) {
            pressedMenu = hit;
            if (hit >= 0) {
                menu = hit;
                sync();
            }
        } else {
            if (hit >= 0 && hit == pressedMenu) {
                menu = hit;
                sync();
                confirm();
            }
            pressedMenu = -1;
        }
    }

    void tick(float dt) {
        animationClock += dt;
        pollInput();
        update(dt);
        updateTitle();
        InvalidateRect(hwnd, nullptr, FALSE);
    }

    void paint(HDC hdc) {
        const int shakeX = shakeTimer > 0.f ? static_cast<int>((rand01() - 0.5f) * shakeStrength * 2.f) : 0;
        const int shakeY = shakeTimer > 0.f ? static_cast<int>((rand01() - 0.5f) * shakeStrength * 2.f) : 0;
        RECT rc{0, 0, W, H};
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
        HGDIOBJ oldBmp = SelectObject(mem, bmp);
        FillRect(mem, &rc, brush(RGB(28, 33, 35)));
        drawBackground(mem);
        if (screen == Screen::Battle || screen == Screen::Result) {
            drawEffects(mem, true);
            drawFighter(mem, ai);
            drawFighter(mem, player);
            drawEffects(mem, false);
            drawParticles(mem);
            drawFloatingTexts(mem);
            drawHud(mem);
            if (screen == Screen::Result) drawResult(mem);
        } else {
            drawParticles(mem);
            drawMenu(mem);
        }
        if (shakeX != 0 || shakeY != 0) {
            HBRUSH black = brush(RGB(5, 5, 5));
            FillRect(hdc, &rc, black);
            DeleteObject(black);
        }
        BitBlt(hdc, shakeX, shakeY, W, H, mem, 0, 0, SRCCOPY);
        SelectObject(mem, oldBmp);
        DeleteObject(bmp);
        DeleteDC(mem);
    }

private:
    Screen screen = Screen::Main;
    Difficulty difficulty = Difficulty::Normal;
    std::mt19937 rng{std::random_device{}()};
    std::vector<Particle> particles;
    std::vector<Effect> effects;
    std::vector<FloatingText> floatingTexts;
    Fighter player;
    Fighter ai;
    Input input;
    Input aiInput;
    int menu = 0;
    int pressedMenu = -1;
    int charSel = 0;
    int weaponSel = 0;
    int diffSel = 1;
    float timeLeft = 99.f;
    float aiThink = 0.f;
    bool mouseDown = false;
    bool lastMouse = false;
    bool practiceMode = false;
    bool visualLabMode = false;
    int visualLabCharacter = 0;
    bool practiceEnemyAttacks = false;
    bool practiceFullEnergy = true;
    bool practiceEnemyFullEnergy = false;
    bool win = false;
    std::wstring message;
    float messageTimer = 0.f;
    float hitStopTimer = 0.f;
    float shakeTimer = 0.f;
    float shakeStrength = 0.f;
    bool koFeedbackPlayed = false;
    float animationClock = 0.f;
    Screen titleScreen = Screen::Main;
    bool titleInitialized = false;
    IWICImagingFactory* wic = nullptr;
    bool comInitialized = false;
    BitmapImage menuBackground;
    BitmapImage battleBackground;
    BitmapImage battleGround;
    std::array<BitmapImage, 3> characterSelectBackgrounds;
    std::array<BitmapImage, 3> weaponSelectBackgrounds;
    std::array<BitmapImage, 3> weaponModelSprites;
    std::array<std::array<BitmapImage, static_cast<size_t>(ActionSpriteSlot::Count)>, 3> characterSprites;
    std::array<BitmapImage, 3> characterUltimateVfx;

    HBRUSH brush(COLORREF color) {
        return CreateSolidBrush(color);
    }

    BitmapImage& characterSprite(int role, ActionSpriteSlot slot) {
        return characterSprites[static_cast<size_t>(std::max(0, std::min(role, 2)))][static_cast<size_t>(slot)];
    }

    BitmapImage& ultimateVfx(int role) {
        return characterUltimateVfx[static_cast<size_t>(std::max(0, std::min(role, 2)))];
    }

    BitmapImage& weaponModelSprite(int weapon) {
        return weaponModelSprites[static_cast<size_t>(std::max(0, std::min(weapon, 2)))];
    }

    ActionSpriteLayout spriteLayoutFor(ActionSpriteSlot slot) const {
        switch (slot) {
        case ActionSpriteSlot::Normal:
            return {260, 220, 12, 48, true};
        case ActionSpriteSlot::Parry:
            return {260, 260, 0, 42, false};
        case ActionSpriteSlot::Charging:
            return {260, 260, 0, 42, true};
        case ActionSpriteSlot::ChargeRelease:
            return {280, 260, 0, 50, true};
        case ActionSpriteSlot::Skill:
            return {560, 320, 140, 64, false};
        case ActionSpriteSlot::Ultimate:
            return {640, 360, 0, 70, false};
        case ActionSpriteSlot::Jump:
        case ActionSpriteSlot::Dodge:
        case ActionSpriteSlot::Hit:
            return {240, 240, 0, 46, true};
        default:
            return {220, 220, 0, 48, true};
        }
    }

    ActionSpriteLayout spriteLayoutFor(ActionSpriteSlot slot, const BitmapImage& image, int role) const {
        ActionSpriteLayout layout = spriteLayoutFor(slot);
        const bool bodyPose =
            slot == ActionSpriteSlot::Idle ||
            slot == ActionSpriteSlot::Run1 ||
            slot == ActionSpriteSlot::Run2 ||
            slot == ActionSpriteSlot::Hit ||
            slot == ActionSpriteSlot::Jump ||
            slot == ActionSpriteSlot::Dodge;
        const bool refinedActionPose =
            role > 0 &&
            (slot == ActionSpriteSlot::Normal ||
             slot == ActionSpriteSlot::Charging ||
             slot == ActionSpriteSlot::ChargeRelease ||
             slot == ActionSpriteSlot::Skill ||
             slot == ActionSpriteSlot::Ultimate);
        if ((bodyPose || refinedActionPose) && image.width > 0 && image.height > 0) {
            const float aspect = static_cast<float>(image.width) / static_cast<float>(image.height);
            if (aspect > 0.25f && aspect < 1.65f && (aspect < 0.85f || aspect > 1.15f)) {
                if (refinedActionPose && (slot == ActionSpriteSlot::Skill || slot == ActionSpriteSlot::Ultimate)) {
                    layout.height = 260;
                } else if (slot == ActionSpriteSlot::Ultimate) {
                    layout.height = 300;
                } else if (slot == ActionSpriteSlot::Skill) {
                    layout.height = 280;
                } else if (refinedActionPose) {
                    layout.height = 260;
                } else {
                    layout.height = role == 0 ? 220 : 240;
                }
                layout.width = static_cast<int>(layout.height * aspect);
                layout.width = std::max(130, std::min(layout.width, refinedActionPose ? 520 : 320));
                layout.xOffset = 0;
                layout.yOffset = refinedActionPose ? 58 : 48;
            }
        }
        if (role == 1 && image.width > 0 && image.height > 0) {
            const float aspect = static_cast<float>(image.width) / static_cast<float>(image.height);
            auto setHeightPreservingAspect = [&](int targetHeight, int minWidth, int maxWidth, int yOffset) {
                layout.height = targetHeight;
                layout.width = static_cast<int>(layout.height * aspect);
                layout.width = std::max(minWidth, std::min(layout.width, maxWidth));
                layout.yOffset = yOffset;
                layout.xOffset = 0;
            };
            if (slot == ActionSpriteSlot::Run1 || slot == ActionSpriteSlot::Run2) {
                setHeightPreservingAspect(240, 156, 230, 52);
            } else if (slot == ActionSpriteSlot::Dodge) {
                setHeightPreservingAspect(240, 156, 230, 52);
            } else if (slot == ActionSpriteSlot::Parry || slot == ActionSpriteSlot::Normal) {
                setHeightPreservingAspect(246, 235, 318, 50);
            } else if (slot == ActionSpriteSlot::Skill) {
                setHeightPreservingAspect(206, 165, 235, 46);
            } else if (
                slot == ActionSpriteSlot::Charging ||
                slot == ActionSpriteSlot::ChargeRelease) {
                setHeightPreservingAspect(218, 170, 250, 48);
            } else if (slot == ActionSpriteSlot::Ultimate) {
                setHeightPreservingAspect(224, 210, 285, 50);
            }
        }
        return layout;
    }

    void initGraphics() {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        comInitialized = SUCCEEDED(hr);
        if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
            CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic));
        }
        loadMenuBackground();
        loadBattleBackground();
        loadArenaGround();
        loadSelectBackgrounds();
        loadAllCharacterActionSprites();
        loadWeaponModelSprites();
    }

    std::wstring exeDir() const {
        wchar_t path[MAX_PATH]{};
        const DWORD len = GetModuleFileNameW(nullptr, path, MAX_PATH);
        std::wstring result(path, path + len);
        const size_t cut = result.find_last_of(L"\\/");
        return cut == std::wstring::npos ? L"." : result.substr(0, cut);
    }

    void loadBattleBackground() {
        const std::wstring path = exeDir() + L"\\assets\\art\\backgrounds\\bamboo_moon\\concept\\bamboo_moon_background_concept.png";
        loadPng(path, battleBackground);
    }

    void loadMenuBackground() {
        const std::wstring path = exeDir() + L"\\assets\\art\\backgrounds\\main_menu\\main_menu_heroes.png";
        loadPng(path, menuBackground);
    }

    void loadArenaGround() {
        const std::wstring path = exeDir() + L"\\assets\\art\\backgrounds\\bamboo_moon\\layers\\ground_strip.png";
        loadPng(path, battleGround);
    }

    void loadSelectBackgrounds() {
        const std::wstring root = exeDir() + L"\\assets\\art\\";
        loadPng(root + L"characters\\lingshuang\\select\\background.png", characterSelectBackgrounds[0]);
        loadPng(root + L"characters\\mohen\\select\\background.png", characterSelectBackgrounds[1]);
        loadPng(root + L"characters\\suxin\\select\\background.png", characterSelectBackgrounds[2]);
        loadPng(root + L"weapons\\longsword\\select\\background.png", weaponSelectBackgrounds[0]);
        loadPng(root + L"weapons\\broadsword\\select\\background.png", weaponSelectBackgrounds[1]);
        loadPng(root + L"weapons\\dualblades\\select\\background.png", weaponSelectBackgrounds[2]);
    }

    void loadAllCharacterActionSprites() {
        for (int role = 0; role < static_cast<int>(CharacterSpriteFolder.size()); ++role) {
            loadCharacterActionSprites(role);
        }
    }

    void loadWeaponModelSprites() {
        const std::wstring root = exeDir() + L"\\assets\\art\\";
        for (int weapon = 0; weapon < static_cast<int>(WeaponModelFolder.size()); ++weapon) {
            loadPng(root + WeaponModelFolder[static_cast<size_t>(weapon)] + L"model_01.png", weaponModelSprite(weapon));
        }
    }

    void loadCharacterActionSprites(int role) {
        const std::wstring dir = exeDir() + L"\\assets\\art\\" + CharacterSpriteFolder[static_cast<size_t>(role)];
        loadPng(dir + L"idle_01.png", characterSprite(role, ActionSpriteSlot::Idle));
        loadPng(dir + L"run_01.png", characterSprite(role, ActionSpriteSlot::Run1));
        loadPng(dir + L"run_02.png", characterSprite(role, ActionSpriteSlot::Run2));
        loadPng(dir + L"jump_01.png", characterSprite(role, ActionSpriteSlot::Jump));
        loadPng(dir + L"dodge_01.png", characterSprite(role, ActionSpriteSlot::Dodge));
        loadPng(dir + L"parry_01.png", characterSprite(role, ActionSpriteSlot::Parry));
        loadPng(dir + L"hit_01.png", characterSprite(role, ActionSpriteSlot::Hit));
        loadPng(dir + L"normal_01.png", characterSprite(role, ActionSpriteSlot::Normal));
        loadPng(dir + L"charging_01.png", characterSprite(role, ActionSpriteSlot::Charging));
        loadPng(dir + L"charge_release_01.png", characterSprite(role, ActionSpriteSlot::ChargeRelease));
        loadPng(dir + L"skill_01.png", characterSprite(role, ActionSpriteSlot::Skill));
        loadPng(dir + L"ultimate_01.png", characterSprite(role, ActionSpriteSlot::Ultimate));
        const std::wstring vfxDir = exeDir() + L"\\assets\\art\\" + CharacterVfxFolder[static_cast<size_t>(role)];
        loadPng(vfxDir + L"ultimate_vfx_01.png", ultimateVfx(role));
    }

    bool loadPng(const std::wstring& path, BitmapImage& out) {
        if (!wic) return false;

        IWICBitmapDecoder* decoder = nullptr;
        IWICBitmapFrameDecode* frame = nullptr;
        IWICFormatConverter* converter = nullptr;
        bool ok = false;

        HRESULT hr = wic->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
        if (SUCCEEDED(hr)) hr = decoder->GetFrame(0, &frame);
        UINT width = 0;
        UINT height = 0;
        if (SUCCEEDED(hr)) hr = frame->GetSize(&width, &height);
        if (SUCCEEDED(hr)) hr = wic->CreateFormatConverter(&converter);
        if (SUCCEEDED(hr)) {
            hr = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.f, WICBitmapPaletteTypeCustom);
        }

        void* bits = nullptr;
        HBITMAP dib = nullptr;
        if (SUCCEEDED(hr) && width > 0 && height > 0) {
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = static_cast<LONG>(width);
            bmi.bmiHeader.biHeight = -static_cast<LONG>(height);
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            dib = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
            if (dib && bits) {
                const UINT stride = width * 4;
                hr = converter->CopyPixels(nullptr, stride, stride * height, static_cast<BYTE*>(bits));
                if (SUCCEEDED(hr)) {
                    out.destroy();
                    out.bitmap = dib;
                    out.width = static_cast<int>(width);
                    out.height = static_cast<int>(height);
                    dib = nullptr;
                    ok = true;
                }
            }
        }

        if (dib) DeleteObject(dib);
        if (converter) converter->Release();
        if (frame) frame->Release();
        if (decoder) decoder->Release();
        return ok;
    }

    void updateTitle() {
        if (titleInitialized && titleScreen == screen) return;
        titleInitialized = true;
        titleScreen = screen;
        const wchar_t* name = L"主菜单";
        if (screen == Screen::Character) name = L"角色选择";
        if (screen == Screen::Weapon) name = L"武器选择";
        if (screen == Screen::Difficulty) name = L"难度选择";
        if (screen == Screen::Practice) name = L"练习设置";
        if (screen == Screen::Battle) name = L"战斗";
        if (screen == Screen::Result) name = L"结算";
        SetWindowTextW(hwnd, (std::wstring(L"墨刃 MVP - ") + name).c_str());
    }

    bool inside(int x, int y, int rx, int ry, int rw, int rh) const {
        return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
    }

    bool commandLineHas(const wchar_t* flag) const {
        const std::wstring cmd = GetCommandLineW();
        return cmd.find(flag) != std::wstring::npos;
    }

    int commandLineInt(const wchar_t* flag, int fallback, int lo, int hi) const {
        const std::wstring cmd = GetCommandLineW();
        const std::wstring key(flag);
        size_t pos = cmd.find(key);
        if (pos == std::wstring::npos) return fallback;
        pos += key.size();
        while (pos < cmd.size() && (cmd[pos] == L' ' || cmd[pos] == L'=' || cmd[pos] == L':')) {
            ++pos;
        }
        if (pos >= cmd.size() || cmd[pos] < L'0' || cmd[pos] > L'9') return fallback;
        int value = 0;
        while (pos < cmd.size() && cmd[pos] >= L'0' && cmd[pos] <= L'9') {
            value = value * 10 + static_cast<int>(cmd[pos] - L'0');
            ++pos;
        }
        return std::max(lo, std::min(value, hi));
    }

    int menuHit(int x, int y) const {
        const int count = optionCount();
        if (screen == Screen::Main) {
            for (int i = 0; i < count; ++i) {
                if (inside(x, y, 480, 450 + i * 66, 320, 48)) {
                    return i;
                }
            }
            return -1;
        }
        for (int i = 0; i < count; ++i) {
            if (inside(x, y, 380, 260 + i * 70, 520, 48)) {
                return i;
            }
        }
        return -1;
    }

    int optionCount() const {
        if (screen == Screen::Main) return 3;
        if (screen == Screen::Character) return 3;
        if (screen == Screen::Weapon) return 3;
        if (screen == Screen::Difficulty) return 3;
        if (screen == Screen::Practice) return 4;
        return 1;
    }

    void sync() {
        if (screen == Screen::Character) charSel = menu;
        if (screen == Screen::Weapon) weaponSel = menu;
        if (screen == Screen::Difficulty) diffSel = menu;
    }

    void confirm() {
        if (screen == Screen::Main) {
            if (menu == 0) {
                practiceMode = false;
                screen = Screen::Character;
                menu = charSel;
            } else if (menu == 1) {
                practiceMode = true;
                screen = Screen::Practice;
                menu = 0;
            } else {
                PostQuitMessage(0);
            }
        } else if (screen == Screen::Character) {
            charSel = menu;
            screen = Screen::Weapon;
            menu = weaponSel;
        } else if (screen == Screen::Weapon) {
            weaponSel = menu;
            screen = Screen::Difficulty;
            menu = diffSel;
        } else if (screen == Screen::Difficulty) {
            diffSel = menu;
            difficulty = static_cast<Difficulty>(diffSel);
            practiceMode = false;
            resetBattle();
            screen = Screen::Battle;
            mouseDown = false;
            lastMouse = false;
        } else if (screen == Screen::Practice) {
            if (menu == 0) {
                practiceEnemyAttacks = !practiceEnemyAttacks;
            } else if (menu == 1) {
                practiceFullEnergy = !practiceFullEnergy;
            } else if (menu == 2) {
                practiceEnemyFullEnergy = !practiceEnemyFullEnergy;
            } else {
                practiceMode = true;
                difficulty = Difficulty::Easy;
                diffSel = 0;
                resetBattle();
                screen = Screen::Battle;
                mouseDown = false;
                lastMouse = false;
            }
        }
    }

    void back() {
        if (screen == Screen::Character) {
            screen = Screen::Main;
            menu = 0;
        } else if (screen == Screen::Weapon) {
            screen = Screen::Character;
            menu = charSel;
        } else if (screen == Screen::Difficulty) {
            screen = Screen::Weapon;
            menu = weaponSel;
        } else if (screen == Screen::Practice) {
            screen = Screen::Main;
            menu = 1;
        }
    }

    Fighter makeFighter(const Character& c, const Weapon& w, Vec p, int facing) {
        Fighter f;
        f.c = c;
        f.w = w;
        f.p = p;
        f.facing = facing;
        f.lastMoveFacing = facing;
        f.hp = c.hp;
        return f;
    }

    void resetBattle() {
        player = makeFighter(Characters[charSel], Weapons[weaponSel], {260.f, 565.f}, 1);
        ai = makeFighter(Characters[(charSel + 1) % 3], Weapons[(weaponSel + 1) % 3], {1020.f, 565.f}, -1);
        timeLeft = 99.f;
        aiThink = 0.f;
        particles.clear();
        effects.clear();
        floatingTexts.clear();
        hitStopTimer = 0.f;
        shakeTimer = 0.f;
        shakeStrength = 0.f;
        koFeedbackPlayed = false;
    }

    void startVisualLab() {
        visualLabMode = true;
        practiceMode = true;
        practiceEnemyAttacks = false;
        practiceFullEnergy = true;
        practiceEnemyFullEnergy = true;
        charSel = visualLabCharacter;
        weaponSel = 0;
        diffSel = 0;
        difficulty = Difficulty::Easy;
        resetBattle();
        screen = Screen::Battle;
        setupVisualLabPose();
        message = L"视觉训练场：1大招  2蓄力释放  3受击  4小技能  5平A  6蓄力";
        messageTimer = 2.2f;
    }

    void setupVisualLabPose() {
        player.p = {360.f, 565.f};
        ai.p = {760.f, 565.f};
        player.facing = 1;
        player.lastMoveFacing = 1;
        ai.facing = -1;
        ai.lastMoveFacing = -1;
        player.hp = player.c.hp;
        ai.hp = ai.c.hp;
        player.energy = 100.f;
        ai.energy = 100.f;
        player.stamina = 100.f;
        ai.stamina = 100.f;
        player.action = Action::Idle;
        ai.action = Action::Idle;
        player.actionTimer = 0.f;
        ai.actionTimer = 0.f;
        player.hitTimer = 0.f;
        ai.hitTimer = 0.f;
        player.chargeTimer = 0.f;
        ai.chargeTimer = 0.f;
        aiInput = {};
        input = {};
        mouseDown = false;
        lastMouse = false;
    }

    void triggerVisualLabAction(VisualLabAction action) {
        particles.clear();
        effects.clear();
        setupVisualLabPose();
        if (action == VisualLabAction::Ultimate) {
            ultimate(player);
        } else if (action == VisualLabAction::ChargeRelease) {
            player.action = Action::ChargeRelease;
            player.actionTimer = 0.46f;
            player.chargeTimer = 1.1f;
            splash({player.p.x + player.facing * 55.f, player.p.y - 30.f}, RGB(15, 15, 15), 18);
            addEffect(EffectType::ChargeRelease, {player.p.x + player.facing * 75.f, player.p.y - 45.f}, player.c.color, 0.46f, 150.f, player.facing, chargeLevel(player.chargeTimer), characterIndex(player));
            message = L"蓄力释放测试";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Hit) {
            player.action = Action::Hit;
            player.actionTimer = 0.48f;
            player.hitTimer = 0.32f;
            player.p.x -= 18.f;
            splash(player.p, ai.c.color, 20);
            addEffect(EffectType::HitBurst, player.p, ai.c.color, 0.42f, 95.f, -player.facing, 1, characterIndex(player));
            message = L"受击测试";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Skill) {
            skill(player);
            message = L"小技能测试";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Normal) {
            player.action = Action::Normal;
            player.actionTimer = 0.38f;
            splash({player.p.x + player.facing * 55.f, player.p.y - 40.f}, player.c.color, 10);
            addEffect(EffectType::Slash, {player.p.x + player.facing * 72.f, player.p.y - 56.f}, player.c.color, 0.34f, 86.f, player.facing, weaponIndex(player), characterIndex(player));
            message = L"\u5e73A\u6d4b\u8bd5";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Charging) {
            player.action = Action::Charging;
            player.actionTimer = 0.72f;
            player.chargeTimer = 0.9f;
            addEffect(EffectType::ChargeLoop, {player.p.x, player.p.y - 42.f}, player.c.color, 0.72f, 72.f, player.facing, chargeLevel(player.chargeTimer), characterIndex(player));
            message = L"\u84c4\u529b\u6d4b\u8bd5";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Jump) {
            player.action = Action::Idle;
            player.actionTimer = 0.f;
            player.airborne = true;
            player.z = 92.f;
            player.vz = 0.f;
            splash({player.p.x, player.p.y - 18.f}, RGB(235, 235, 225), 8);
            message = L"\u8df3\u8dc3\u6d4b\u8bd5";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Parry) {
            player.action = Action::Parry;
            player.actionTimer = player.w.parryWindow;
            player.facing = player.lastMoveFacing;
            addEffect(EffectType::ParryGuard, player.p, RGB(245, 245, 235), 0.24f, 80.f, player.facing, 0);
            message = L"\u632f\u5200\u6d4b\u8bd5";
            messageTimer = 0.8f;
        } else if (action == VisualLabAction::Dodge) {
            player.action = Action::Dodge;
            player.actionTimer = 0.28f;
            player.facing = player.lastMoveFacing;
            addEffect(EffectType::DodgeTrail, player.p, player.c.color, 0.28f, 76.f, player.facing, 0);
            message = L"\u51b2\u523a\u6d4b\u8bd5";
            messageTimer = 0.8f;
        }
    }

    bool key(int k) const {
        return (GetAsyncKeyState(k) & 0x8000) != 0;
    }

    void pollInput() {
        input = {};
        if (key('A')) input.move.x -= 1.f;
        if (key('D')) input.move.x += 1.f;
        if (key('W')) input.move.y -= 1.f;
        if (key('S')) input.move.y += 1.f;
        input.move = norm(input.move);
        input.parry = key('G');
        input.dodge = key(VK_LSHIFT) || key(VK_RSHIFT);
        input.skill = key('F');
        input.ultimate = key('V');
        input.jump = key(VK_SPACE);
        input.attackHeld = mouseDown;
        input.attackPress = mouseDown && !lastMouse;
        input.attackRelease = !mouseDown && lastMouse;
        lastMouse = mouseDown;
    }

    void update(float dt) {
        updateFeedback(dt);
        updateParticles(dt);
        updateEffects(dt);
        if (messageTimer > 0.f) messageTimer -= dt;
        if (screen != Screen::Battle) return;
        if (hitStopTimer > 0.f) return;
        if (practiceMode) {
            timeLeft = 99.f;
        } else {
            timeLeft -= dt;
        }
        if (!practiceMode || practiceEnemyAttacks) {
            updateAi(dt);
        } else {
            aiInput = {};
        }
        if (practiceMode && practiceFullEnergy) player.energy = 100.f;
        if (practiceMode && practiceEnemyFullEnergy) ai.energy = 100.f;
        updateFighter(player, input, dt);
        updateFighter(ai, aiInput, dt);
        if (practiceMode && practiceFullEnergy) player.energy = 100.f;
        if (practiceMode && practiceEnemyFullEnergy) ai.energy = 100.f;
        resolve(player, ai);
        resolve(ai, player);
        finishBattleIfOver();
    }

    void finishBattleIfOver() {
        if (!practiceMode && (player.hp <= 0.f || ai.hp <= 0.f || timeLeft <= 0.f)) {
            win = player.hp >= ai.hp;
            triggerKoFeedback();
            screen = Screen::Result;
        }
    }

public:
    bool runDeathSelfTest() {
        // Suxin's passive may heal non-lethal hits, but it must not revive her from a lethal hit.
        practiceMode = false;
        charSel = 2;
        weaponSel = 0;
        resetBattle();
        screen = Screen::Battle;
        player.hp = 30.f;
        ai.p = {player.p.x - 80.f, player.p.y};
        ai.facing = 1;
        ai.action = Action::Ultimate;
        ai.actionTimer = 0.75f;
        resolve(ai, player);
        finishBattleIfOver();
        const bool suxinPlayerDies = player.hp <= 0.f && screen == Screen::Result && !win;

        charSel = 1;
        weaponSel = 0;
        resetBattle();
        screen = Screen::Battle;
        ai.hp = 30.f;
        player.p = {ai.p.x - 80.f, ai.p.y};
        player.facing = 1;
        player.action = Action::Normal;
        player.actionTimer = 0.35f;
        player.comboIndex = 1;
        resolve(player, ai);
        finishBattleIfOver();
        const bool suxinEnemyDies = ai.hp <= 0.f && screen == Screen::Result && win;

        return suxinPlayerDies && suxinEnemyDies;
    }

private:
    bool freeToAct(const Fighter& f) const {
        return f.action == Action::Idle || f.action == Action::Dodge;
    }

    void updateFighter(Fighter& f, const Input& in, float dt) {
        f.cooldown = std::max(0.f, f.cooldown - dt);
        f.actionTimer = std::max(0.f, f.actionTimer - dt);
        f.hitTimer = std::max(0.f, f.hitTimer - dt);
        f.comboTimer = std::max(0.f, f.comboTimer - dt);
        f.dodgeLock = std::max(0.f, f.dodgeLock - dt);
        f.bindTimer = std::max(0.f, f.bindTimer - dt);
        f.slowTimer = std::max(0.f, f.slowTimer - dt);
        f.lotusTimer = std::max(0.f, f.lotusTimer - dt);
        if (f.actionTimer <= 0.f && f.action != Action::Charging && f.action != Action::Down) f.action = Action::Idle;
        if (f.hp <= 0.f) {
            f.action = Action::Down;
            return;
        }
        f.energy = clampf(f.energy + 2.f * dt, 0.f, 100.f);
        f.stamina = clampf(f.stamina + f.c.staminaRegen * dt, 0.f, 100.f);
        if (f.lotusTimer > 0.f && f.c.name == Characters[2].name) f.hp = clampf(f.hp + f.c.hp * 0.4f / 3.f * dt, 0.f, f.c.hp);

        if (f.airborne) {
            f.vz -= 700.f * dt;
            f.z += f.vz * dt;
            if (f.z <= 0.f) {
                f.z = 0.f;
                f.vz = 0.f;
                f.airborne = false;
            }
        } else if (in.jump && freeToAct(f)) {
            f.airborne = true;
            f.vz = 410.f;
        }

        if (in.parry && freeToAct(f)) {
            f.facing = f.lastMoveFacing;
            f.action = Action::Parry;
            f.actionTimer = f.w.parryWindow;
            splash(f.p, RGB(245, 245, 235), 12);
            addEffect(EffectType::ParryGuard, f.p, RGB(245, 245, 235), 0.22f, 80.f, f.facing, 0);
        } else if (in.dodge && freeToAct(f) && f.stamina >= 25.f && f.dodgeLock <= 0.f && f.bindTimer <= 0.f) {
            Vec d = len(in.move) < 0.1f ? Vec{static_cast<float>(-f.facing), 0.f} : in.move;
            addEffect(EffectType::DodgeTrail, f.p, f.c.color, 0.28f, 76.f, f.facing, 0);
            f.p.x += norm(d).x * 120.f;
            f.p.y += norm(d).y * 120.f;
            f.stamina -= 25.f;
            f.dodgeLock = 0.5f;
            f.action = Action::Dodge;
            f.actionTimer = 0.18f;
            f.dodges++;
            splash(f.p, RGB(235, 235, 225), 14);
        } else if (in.skill && (freeToAct(f) || f.airborne) && f.cooldown <= 0.f) {
            skill(f);
        } else if (in.ultimate && (freeToAct(f) || f.airborne) && f.energy >= 100.f) {
            ultimate(f);
        } else if (in.attackPress && freeToAct(f)) {
            f.action = Action::Charging;
            f.chargeTimer = 0.f;
        } else if (in.attackHeld && f.action == Action::Charging) {
            f.chargeTimer += dt;
            const int lvl = chargeLevel(f.chargeTimer);
            addEffect(EffectType::ChargeLoop, {f.p.x, f.p.y - 42.f}, f.c.color, 0.16f, 54.f + 16.f * lvl, f.facing, lvl, characterIndex(f));
        } else if (in.attackRelease && f.action == Action::Charging) {
            if (f.chargeTimer < QuickAttackThreshold) {
                normal(f);
            } else {
                f.action = Action::ChargeRelease;
                f.actionTimer = 0.28f;
                splash({f.p.x + f.facing * 55.f, f.p.y - 30.f}, RGB(15, 15, 15), 18);
                addEffect(EffectType::ChargeRelease, {f.p.x + f.facing * 75.f, f.p.y - 45.f}, f.c.color, 0.36f, 115.f + 30.f * chargeLevel(f.chargeTimer), f.facing, chargeLevel(f.chargeTimer), characterIndex(f));
            }
        } else if (in.attackPress && f.comboTimer > 0.f) {
            normal(f);
        }

        if (freeToAct(f) && f.bindTimer <= 0.f) {
            const float slow = f.slowTimer > 0.f ? 0.7f : 1.f;
            f.p.x += in.move.x * f.c.speed * slow * dt;
            f.p.y += in.move.y * f.c.speed * slow * dt;
        }
        if (absf(in.move.x) > 0.1f) {
            f.facing = in.move.x > 0.f ? 1 : -1;
            f.lastMoveFacing = f.facing;
        }
        clampFighterToArena(f);
    }

    void clampFighterToArena(Fighter& f) {
        f.p.x = clampf(f.p.x, ArenaLeft, ArenaRight);
        f.p.y = clampf(f.p.y, ArenaTop, ArenaBottom);
    }

    void normal(Fighter& f) {
        f.action = Action::Normal;
        f.actionTimer = 0.2f + 0.03f * f.comboIndex;
        f.comboTimer = 0.32f;
        f.comboIndex = (f.comboIndex + 1) % static_cast<int>(f.w.combo.size());
        f.maxCombo = std::max(f.maxCombo, f.comboIndex + 1);
        splash({f.p.x + f.facing * 55.f, f.p.y - 40.f}, f.c.color, 10);
        const int level = f.w.name == Weapons[1].name ? 2 : (f.w.name == Weapons[2].name ? 1 : 0);
        addEffect(EffectType::Slash, {f.p.x + f.facing * 72.f, f.p.y - 56.f}, f.c.color, 0.24f, 86.f, f.facing, level, characterIndex(f));
    }

    void skill(Fighter& f) {
        f.action = Action::Skill;
        f.actionTimer = 0.3f;
        f.cooldown = f.c.name == Characters[1].name ? 4.8f : (f.c.name == Characters[0].name ? 8.f : 10.f);
        if (f.c.name == Characters[0].name) f.p.x += f.facing * 170.f;
        if (f.c.name == Characters[2].name) f.hp = clampf(f.hp + f.c.hp * 0.2f, 0.f, f.c.hp);
        splash(f.p, f.c.color, 24);
        addEffect(EffectType::SkillAura, f.p, f.c.color, 0.55f, 100.f, f.facing, 0, characterIndex(f));
    }

    void ultimate(Fighter& f) {
        f.action = Action::Ultimate;
        f.actionTimer = 0.75f;
        f.energy = 0.f;
        if (f.c.name == Characters[2].name) f.lotusTimer = 3.f;
        message = std::wstring(f.c.name) + L" 释放大招";
        messageTimer = 1.2f;
        splash(f.p, f.c.color, 42);
        addEffect(EffectType::UltimateAura, f.p, f.c.color, 0.85f, 170.f, f.facing, 1, characterIndex(f));
    }

    int chargeLevel(float t) const {
        return t >= 1.f ? 2 : (t >= 0.5f ? 1 : 0);
    }

    void resolve(Fighter& a, Fighter& d) {
        if (a.actionTimer <= 0.f || d.hitTimer > 0.f || d.action == Action::Down) return;
        float range = 0.f;
        float mult = 0.f;
        bool charge = false;
        bool control = false;
        if (a.action == Action::Normal) {
            range = 95.f;
            int idx = (a.comboIndex - 1 + static_cast<int>(a.w.combo.size())) % static_cast<int>(a.w.combo.size());
            mult = a.w.combo[idx];
        } else if (a.action == Action::ChargeRelease) {
            range = a.w.name == Weapons[0].name ? 360.f : (a.w.name == Weapons[2].name ? 230.f : 130.f);
            int lvl = chargeLevel(a.chargeTimer);
            mult = a.w.charge[lvl] * (lvl == 0 ? 1.f : (lvl == 1 ? 1.2f : 1.5f));
            charge = true;
        } else if (a.action == Action::Counter) {
            range = 120.f;
            mult = a.w.counter;
        } else if (a.action == Action::Skill) {
            range = a.c.name == Characters[1].name ? 210.f : 105.f;
            mult = a.c.name == Characters[2].name ? 0.f : 0.9f;
            control = true;
        } else if (a.action == Action::Ultimate) {
            range = a.c.name == Characters[1].name ? 390.f : 150.f;
            mult = a.c.name == Characters[0].name ? 3.5f : (a.c.name == Characters[1].name ? 1.5f : 0.f);
            control = true;
        } else {
            return;
        }

        const float dx = d.p.x - a.p.x;
        const bool front = a.facing > 0 ? dx > -30.f : dx < 30.f;
        if (!front || absf(dx) > range || absf(d.p.y - a.p.y) > 90.f) return;

        if (charge && d.action == Action::Parry && d.actionTimer > 0.f) {
            a.action = Action::Hit;
            a.actionTimer = 0.35f;
            d.action = Action::Counter;
            d.actionTimer = 0.28f;
            d.parries++;
            d.flashTimer = 0.12f;
            message = L"\u632f\u5200\u6210\u529f";
            messageTimer = 1.f;
            addFloatingText({d.p.x, d.p.y - 162.f}, L"\u632f\u5200", RGB(245, 245, 235), 34);
            addHitStop(0.12f, 10.f);
            splash(d.p, RGB(245, 245, 235), 32);
            addEffect(EffectType::ParrySuccess, d.p, RGB(245, 245, 235), 0.52f, 145.f, d.facing, 0);
            addEffect(EffectType::HitBurst, a.p, RGB(160, 160, 160), 0.3f, 90.f, -d.facing, 1);
            return;
        }
        if (a.action == Action::Normal && (d.action == Action::Charging || d.action == Action::Parry)) {
            if (d.action == Action::Parry) {
                addEffect(EffectType::ParryBreak, d.p, RGB(145, 145, 145), 0.34f, 90.f, d.facing, 0);
            }
            d.action = Action::Hit;
            d.actionTimer = 0.35f;
            message = L"\u5e73A\u7834\u62db";
            messageTimer = 0.75f;
            addFloatingText({d.p.x, d.p.y - 164.f}, L"\u7834\u62db", RGB(245, 235, 165), 28);
        }

        float damage = BaseDamage * mult * a.c.attack;
        if (d.lotusTimer > 0.f) damage *= 0.7f;
        const float hpBeforeDamage = d.hp;
        d.hp = clampf(d.hp - damage, 0.f, d.c.hp);
        const bool defeatedByThisHit = hpBeforeDamage > 0.f && d.hp <= 0.f;
        a.damageDone += damage;
        a.energy = clampf(a.energy + 8.f * a.c.hitEnergy, 0.f, 100.f);
        d.energy = clampf(d.energy + 5.f * d.c.takenEnergy, 0.f, 100.f);
        if (!defeatedByThisHit && d.c.name == Characters[2].name) d.hp = clampf(d.hp + damage * 0.05f, 0.f, d.c.hp);
        d.hitTimer = 0.25f;
        d.action = Action::Hit;
        d.actionTimer = 0.25f;
        d.p.x += a.facing * (charge ? 45.f : 18.f);
        if (charge) a.chargeHits++;
        if (control) {
            if (a.c.name == Characters[0].name) d.slowTimer = 2.f;
            if (a.c.name == Characters[1].name) d.bindTimer = a.action == Action::Ultimate ? 2.f : 1.f;
        }
        splash(d.p, a.c.color, 20);
        addEffect(EffectType::HitBurst, d.p, charge ? RGB(205, 35, 45) : a.c.color, charge ? 0.42f : 0.28f, charge ? 120.f : 70.f, a.facing, charge ? 2 : 0, characterIndex(a));
        addHitFeedback(a, d, damage, charge, control, defeatedByThisHit);
    }

    float rand01() {
        return std::uniform_real_distribution<float>(0.f, 1.f)(rng);
    }

    void updateAi(float dt) {
        aiThink -= dt;
        aiInput = {};
        const float dist = absf(player.p.x - ai.p.x);
        const float react = difficulty == Difficulty::Easy ? 0.8f : (difficulty == Difficulty::Normal ? 0.45f : 0.18f);
        if (aiThink <= 0.f) {
            aiThink = react + rand01() * react * 0.5f;
            const float parry = difficulty == Difficulty::Easy ? 0.1f : (difficulty == Difficulty::Normal ? 0.35f : 0.7f);
            const float dodge = difficulty == Difficulty::Easy ? 0.1f : (difficulty == Difficulty::Normal ? 0.4f : 0.8f);
            const float skillRate = difficulty == Difficulty::Easy ? 0.3f : (difficulty == Difficulty::Normal ? 0.7f : 0.95f);
            if (player.action == Action::ChargeRelease && rand01() < parry) aiInput.parry = true;
            else if (dist < 140.f && player.action == Action::Normal && rand01() < dodge) aiInput.dodge = true;
            else if (ai.energy >= 100.f && rand01() < skillRate) aiInput.ultimate = true;
            else if (ai.cooldown <= 0.f && rand01() < skillRate * 0.35f) aiInput.skill = true;
            else if (dist < 150.f) {
                if (rand01() < 0.55f) aiInput.attackPress = true;
                else {
                    ai.action = Action::Charging;
                    ai.chargeTimer = difficulty == Difficulty::Hard ? 1.1f : 0.55f;
                    aiInput.attackRelease = true;
                }
            }
        }
        if (dist > 130.f) aiInput.move.x = player.p.x > ai.p.x ? 1.f : -1.f;
        else if (dist < 90.f) aiInput.move.x = player.p.x > ai.p.x ? -1.f : 1.f;
        aiInput.move.y = clampf((player.p.y - ai.p.y) / 120.f, -1.f, 1.f);
        aiInput.move = norm(aiInput.move);
    }

    void splash(Vec p, COLORREF c, int count) {
        for (int i = 0; i < count; ++i) {
            const float a = rand01() * 6.28318f;
            const float s = 40.f + rand01() * 210.f;
            particles.push_back({p, {std::cos(a) * s, std::sin(a) * s}, c, 0.45f + rand01() * 0.45f, 0.9f, 2.f + rand01() * 5.f});
        }
    }

    void addFloatingText(Vec p, const std::wstring& value, COLORREF color, int size = 28) {
        floatingTexts.push_back({p, {-18.f + rand01() * 36.f, -92.f - rand01() * 34.f}, value, color, 0.82f, 0.82f, size});
    }

    void addHitStop(float seconds, float shake) {
        hitStopTimer = std::max(hitStopTimer, seconds);
        shakeTimer = std::max(shakeTimer, seconds + 0.08f);
        shakeStrength = std::max(shakeStrength, shake);
    }

    void addHitFeedback(Fighter& attacker, Fighter& defender, float damage, bool charge, bool control, bool defeated) {
        defender.flashTimer = 0.16f;
        const int shownDamage = static_cast<int>(std::ceil(std::max(0.f, damage)));
        addFloatingText({defender.p.x + attacker.facing * 20.f, defender.p.y - 132.f}, L"-" + std::to_wstring(shownDamage), charge ? RGB(255, 225, 105) : RGB(245, 245, 235), charge ? 34 : 28);
        if (charge) {
            addFloatingText({defender.p.x, defender.p.y - 168.f}, L"\u84c4\u529b\u547d\u4e2d", RGB(255, 205, 80), 24);
            message = L"\u84c4\u529b\u547d\u4e2d";
            messageTimer = 0.75f;
        } else if (attacker.action == Action::Counter) {
            addFloatingText({defender.p.x, defender.p.y - 168.f}, L"\u53cd\u51fb", RGB(245, 245, 235), 24);
            message = L"\u632f\u5200\u53cd\u51fb";
            messageTimer = 0.75f;
        } else if (control) {
            addFloatingText({defender.p.x, defender.p.y - 168.f}, L"\u63a7\u5236", attacker.c.color, 24);
        }
        addHitStop(defeated ? 0.18f : (charge ? 0.11f : 0.055f), defeated ? 13.f : (charge ? 9.f : 4.f));
    }

    void triggerKoFeedback() {
        if (koFeedbackPlayed) return;
        koFeedbackPlayed = true;
        const Vec p = player.hp <= 0.f ? player.p : ai.p;
        splash({p.x, p.y - 70.f}, RGB(245, 235, 210), 52);
        addEffect(EffectType::HitBurst, {p.x, p.y - 44.f}, RGB(245, 235, 210), 0.62f, 150.f, 1, 2);
        addFloatingText({W * 0.5f, 230.f}, L"KO", RGB(245, 235, 210), 72);
        message = win ? L"\u80dc\u5229" : L"\u6218\u8d25";
        messageTimer = 1.4f;
        addHitStop(0.22f, 16.f);
    }

    void addEffect(EffectType type, Vec p, COLORREF c, float life, float radius, int facing, int level, int role = -1) {
        effects.push_back({type, p, c, life, life, radius, facing, level, role});
    }

    int characterIndex(const Fighter& f) const {
        for (int i = 0; i < static_cast<int>(Characters.size()); ++i) {
            if (f.c.name == Characters[static_cast<size_t>(i)].name) return i;
        }
        return 0;
    }

    int weaponIndex(const Fighter& f) const {
        for (int i = 0; i < static_cast<int>(Weapons.size()); ++i) {
            if (f.w.name == Weapons[static_cast<size_t>(i)].name) return i;
        }
        return 0;
    }

    void updateEffects(float dt) {
        for (auto& e : effects) {
            e.life -= dt;
        }
        effects.erase(std::remove_if(effects.begin(), effects.end(), [](const Effect& e) { return e.life <= 0.f; }), effects.end());
    }

    void updateFeedback(float dt) {
        hitStopTimer = std::max(0.f, hitStopTimer - dt);
        shakeTimer = std::max(0.f, shakeTimer - dt);
        if (shakeTimer <= 0.f) shakeStrength = 0.f;
        player.flashTimer = std::max(0.f, player.flashTimer - dt);
        ai.flashTimer = std::max(0.f, ai.flashTimer - dt);
        for (auto& f : floatingTexts) {
            f.life -= dt;
            f.p.x += f.v.x * dt;
            f.p.y += f.v.y * dt;
            f.v.y += 95.f * dt;
        }
        floatingTexts.erase(std::remove_if(floatingTexts.begin(), floatingTexts.end(), [](const FloatingText& f) { return f.life <= 0.f; }), floatingTexts.end());
    }

    void updateParticles(float dt) {
        static float ambient = 0.f;
        ambient += dt;
        if (ambient > 0.08f) {
            ambient = 0.f;
            particles.push_back({{rand01() * W, 80.f + rand01() * 260.f}, {-15.f + rand01() * 30.f, 25.f + rand01() * 35.f}, RGB(95, 150, 105), 5.f, 5.f, 2.f});
        }
        for (auto& p : particles) {
            p.life -= dt;
            p.p.x += p.v.x * dt;
            p.p.y += p.v.y * dt;
            p.v.x *= 0.96f;
            p.v.y *= 0.96f;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return p.life <= 0.f; }), particles.end());
    }

    void rect(HDC hdc, int x, int y, int w, int h, COLORREF color) {
        HBRUSH b = brush(color);
        const int x2 = x + w;
        const int y2 = y + h;
        RECT r{std::min(x, x2), std::min(y, y2), std::max(x, x2), std::max(y, y2)};
        FillRect(hdc, &r, b);
        DeleteObject(b);
    }

    void ellipse(HDC hdc, int x, int y, int w, int h, COLORREF fill, COLORREF penColor = RGB(20, 20, 20), int penW = 1) {
        HBRUSH b = brush(fill);
        HPEN p = CreatePen(PS_SOLID, penW, penColor);
        HGDIOBJ ob = SelectObject(hdc, b);
        HGDIOBJ op = SelectObject(hdc, p);
        Ellipse(hdc, x, y, x + w, y + h);
        SelectObject(hdc, ob);
        SelectObject(hdc, op);
        DeleteObject(b);
        DeleteObject(p);
    }

    void fillPoly(HDC hdc, const std::vector<POINT>& pts, COLORREF fill, COLORREF outline = RGB(20, 20, 20), int width = 1) {
        if (pts.size() < 3) return;
        HBRUSH b = brush(fill);
        HPEN p = CreatePen(PS_SOLID, width, outline);
        HGDIOBJ ob = SelectObject(hdc, b);
        HGDIOBJ op = SelectObject(hdc, p);
        Polygon(hdc, pts.data(), static_cast<int>(pts.size()));
        SelectObject(hdc, ob);
        SelectObject(hdc, op);
        DeleteObject(b);
        DeleteObject(p);
    }

    void stroke(HDC hdc, const std::vector<POINT>& pts, COLORREF color, int width) {
        if (pts.size() < 2) return;
        HPEN pen = CreatePen(PS_SOLID, width, color);
        HGDIOBJ old = SelectObject(hdc, pen);
        Polyline(hdc, pts.data(), static_cast<int>(pts.size()));
        SelectObject(hdc, old);
        DeleteObject(pen);
    }

    void line(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color, int width) {
        HPEN pen = CreatePen(PS_SOLID, width, color);
        HGDIOBJ old = SelectObject(hdc, pen);
        MoveToEx(hdc, x1, y1, nullptr);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, old);
        DeleteObject(pen);
    }

    void text(HDC hdc, const std::wstring& s, int x, int y, int size, COLORREF color, bool center = false) {
        HFONT font = CreateFontW(size, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
        HGDIOBJ old = SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, color);
        RECT r{x, y, center ? W : W - 20, y + size + 12};
        DrawTextW(hdc, s.c_str(), -1, &r, center ? DT_CENTER : DT_LEFT);
        SelectObject(hdc, old);
        DeleteObject(font);
    }

    void textInRect(HDC hdc, const std::wstring& s, RECT r, int size, COLORREF color) {
        HFONT font = CreateFontW(size, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Microsoft YaHei UI");
        HGDIOBJ old = SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, color);
        DrawTextW(hdc, s.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, old);
        DeleteObject(font);
    }

    void drawBackground(HDC hdc) {
        if (screen == Screen::Main && drawMainMenuBackgroundImage(hdc)) {
            return;
        }
        if (screen == Screen::Character && drawSelectBackgroundImage(hdc, characterSelectBackgrounds[static_cast<size_t>(std::max(0, std::min(menu, 2)))])) {
            return;
        }
        if (screen == Screen::Weapon && drawSelectBackgroundImage(hdc, weaponSelectBackgrounds[static_cast<size_t>(std::max(0, std::min(menu, 2)))])) {
            return;
        }
        if ((screen == Screen::Difficulty || screen == Screen::Practice) && drawBattleBackgroundImage(hdc)) {
            return;
        }
        if ((screen == Screen::Battle || screen == Screen::Result) && drawBattleBackgroundImage(hdc)) {
            return;
        }
        rect(hdc, 0, 0, W, H, RGB(30, 36, 39));
        ellipse(hdc, 940, 70, 116, 116, RGB(218, 221, 200), RGB(218, 221, 200));
        for (int i = 0; i < 22; ++i) rect(hdc, i * 65 + 10, 160, 7, 430, i % 2 ? RGB(38, 70, 58) : RGB(45, 82, 65));
        rect(hdc, 0, 545, W, 175, RGB(58, 63, 60));
        for (int i = 0; i < 13; ++i) rect(hdc, 0, 560 + i * 12, W, 1, RGB(83, 86, 80));
    }

    bool drawBattleBackgroundImage(HDC hdc) {
        if (!battleBackground.bitmap || battleBackground.width <= 0 || battleBackground.height <= 0) return false;

        HDC src = CreateCompatibleDC(hdc);
        if (!src) return false;
        HGDIOBJ old = SelectObject(src, battleBackground.bitmap);

        int sx = 0;
        int sy = 0;
        int sw = battleBackground.width;
        int sh = battleBackground.height;
        const float srcAspect = static_cast<float>(battleBackground.width) / static_cast<float>(battleBackground.height);
        const float dstAspect = static_cast<float>(W) / static_cast<float>(H);
        if (srcAspect > dstAspect) {
            sw = static_cast<int>(battleBackground.height * dstAspect);
            sx = (battleBackground.width - sw) / 2;
        } else {
            sh = static_cast<int>(battleBackground.width / dstAspect);
            sy = static_cast<int>((battleBackground.height - sh) * 0.42f);
        }

        SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, 0, 0, W, H, src, sx, sy, sw, sh, SRCCOPY);

        SelectObject(src, old);
        DeleteDC(src);
        return true;
    }

    bool drawMainMenuBackgroundImage(HDC hdc) {
        if (!menuBackground.bitmap || menuBackground.width <= 0 || menuBackground.height <= 0) return false;

        HDC src = CreateCompatibleDC(hdc);
        if (!src) return false;
        HGDIOBJ old = SelectObject(src, menuBackground.bitmap);

        int sx = 0;
        int sy = 0;
        int sw = menuBackground.width;
        int sh = menuBackground.height;
        const float srcAspect = static_cast<float>(menuBackground.width) / static_cast<float>(menuBackground.height);
        const float dstAspect = static_cast<float>(W) / static_cast<float>(H);
        if (srcAspect > dstAspect) {
            sw = static_cast<int>(menuBackground.height * dstAspect);
            sx = (menuBackground.width - sw) / 2;
        } else {
            sh = static_cast<int>(menuBackground.width / dstAspect);
            sy = (menuBackground.height - sh) / 2;
        }

        SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, 0, 0, W, H, src, sx, sy, sw, sh, SRCCOPY);
        SelectObject(src, old);
        DeleteDC(src);
        return true;
    }

    bool drawSelectBackgroundImage(HDC hdc, const BitmapImage& image) {
        if (!image.bitmap || image.width <= 0 || image.height <= 0) return false;

        HDC src = CreateCompatibleDC(hdc);
        if (!src) return false;
        HGDIOBJ old = SelectObject(src, image.bitmap);

        int sx = 0;
        int sy = 0;
        int sw = image.width;
        int sh = image.height;
        const float srcAspect = static_cast<float>(image.width) / static_cast<float>(image.height);
        const float dstAspect = static_cast<float>(W) / static_cast<float>(H);
        if (srcAspect > dstAspect) {
            sw = static_cast<int>(image.height * dstAspect);
            sx = (image.width - sw) / 2;
        } else {
            sh = static_cast<int>(image.width / dstAspect);
            sy = (image.height - sh) / 2;
        }

        SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, 0, 0, W, H, src, sx, sy, sw, sh, SRCCOPY);
        SelectObject(src, old);
        DeleteDC(src);
        return true;
    }

    bool drawArenaGround(HDC hdc) {
        const int y = static_cast<int>(ArenaTop - 16.f);
        const int height = H - y;
        if (!battleGround.bitmap || battleGround.width <= 0 || battleGround.height <= 0) {
            rect(hdc, 0, y, W, height, RGB(38, 44, 40));
            for (int i = 0; i < 8; ++i) {
                line(hdc, 0, y + 22 + i * 22, W, y + 16 + i * 24, RGB(78, 88, 78), 1);
            }
            return false;
        }

        HDC src = CreateCompatibleDC(hdc);
        if (!src) return false;
        HGDIOBJ old = SelectObject(src, battleGround.bitmap);
        SetStretchBltMode(hdc, HALFTONE);
        SetBrushOrgEx(hdc, 0, 0, nullptr);
        StretchBlt(hdc, 0, y, W, height, src, 0, 0, battleGround.width, battleGround.height, SRCCOPY);
        SelectObject(src, old);
        DeleteDC(src);
        return true;
    }

    bool drawBitmapAlpha(HDC hdc, const BitmapImage& image, int x, int y, int width, int height, bool mirror = false, BYTE alpha = 255) {
        if (!image.bitmap || image.width <= 0 || image.height <= 0 || width <= 0 || height <= 0) return false;

        HDC src = CreateCompatibleDC(hdc);
        if (!src) return false;
        HGDIOBJ old = SelectObject(src, image.bitmap);
        BLENDFUNCTION blend{};
        blend.BlendOp = AC_SRC_OVER;
        blend.SourceConstantAlpha = alpha;
        blend.AlphaFormat = AC_SRC_ALPHA;
        BOOL ok = FALSE;
        if (mirror) {
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = image.width;
            bmi.bmiHeader.biHeight = -image.height;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            void* bits = nullptr;
            HBITMAP flipped = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
            HDC flipDc = CreateCompatibleDC(hdc);
            if (flipped && flipDc) {
                HGDIOBJ oldFlip = SelectObject(flipDc, flipped);
                StretchBlt(flipDc, 0, 0, image.width, image.height, src, image.width - 1, 0, -image.width, image.height, SRCCOPY);
                ok = AlphaBlend(hdc, x, y, width, height, flipDc, 0, 0, image.width, image.height, blend);
                SelectObject(flipDc, oldFlip);
            }
            if (flipDc) DeleteDC(flipDc);
            if (flipped) DeleteObject(flipped);
        } else {
            ok = AlphaBlend(hdc, x, y, width, height, src, 0, 0, image.width, image.height, blend);
        }
        SelectObject(src, old);
        DeleteDC(src);
        return ok == TRUE;
    }

    void drawParticles(HDC hdc) {
        for (const auto& p : particles) {
            const float k = clampf(p.life / p.maxLife, 0.f, 1.f);
            int r = static_cast<int>(p.r * k);
            if (r > 0) ellipse(hdc, static_cast<int>(p.p.x) - r, static_cast<int>(p.p.y) - r, r * 2, r * 2, p.color, p.color);
        }
    }

    void drawFloatingTexts(HDC hdc) {
        for (const auto& f : floatingTexts) {
            const float k = clampf(f.life / f.maxLife, 0.f, 1.f);
            const int size = std::max(16, static_cast<int>(f.size * (0.86f + 0.22f * k)));
            const int x = static_cast<int>(f.p.x);
            const int y = static_cast<int>(f.p.y);
            if (f.text == L"KO") {
                text(hdc, f.text, 0, y + 3, size, RGB(20, 20, 20), true);
                text(hdc, f.text, 0, y, size, f.color, true);
            } else {
                text(hdc, f.text, x + 2, y + 2, size, RGB(18, 18, 18));
                text(hdc, f.text, x, y, size, f.color);
            }
        }
    }

    void drawRoleSlash(HDC hdc, const Effect& e, int x, int y, int r) {
        const int role = e.role < 0 ? 0 : e.role;
        const int wide = e.level == 2 ? 10 : (e.level == 1 ? 5 : 4);
        const int bands = e.level == 1 ? 2 : 1;
        if (role == 0) {
            for (int band = 0; band < bands; ++band) {
                std::vector<POINT> pts;
                for (int i = 0; i <= 14; ++i) {
                    const float u = static_cast<float>(i) / 14.f;
                    const float arc = -1.1f + u * 2.2f;
                    const float px = e.p.x + e.facing * (std::cos(arc) * r * 0.82f + 18.f * band);
                    const float py = e.p.y + std::sin(arc) * r * 0.48f + (u - 0.5f) * 32.f;
                    pts.push_back({static_cast<LONG>(px), static_cast<LONG>(py)});
                }
                stroke(hdc, pts, band == 0 ? RGB(238, 252, 255) : RGB(115, 205, 255), wide);
            }
            for (int i = -2; i <= 2; ++i) {
                line(hdc, x - e.facing * 8, y + i * 13, x + e.facing * (r / 2 + i * 8), y - 30 + i * 6, RGB(180, 235, 255), 2);
            }
        } else if (role == 1) {
            std::vector<POINT> pts;
            for (int i = 0; i <= 16; ++i) {
                const float u = static_cast<float>(i) / 16.f;
                const float px = e.p.x + e.facing * (u * r * 1.08f - r * 0.24f);
                const float py = e.p.y - 38.f + std::sin(u * 3.14159f) * r * 0.72f;
                pts.push_back({static_cast<LONG>(px), static_cast<LONG>(py)});
            }
            stroke(hdc, pts, RGB(18, 14, 25), wide + 5);
            stroke(hdc, pts, RGB(165, 105, 230), std::max(3, wide - 1));
            for (int i = 0; i < 6; ++i) {
                const int dx = e.facing * (24 + i * 19);
                line(hdc, x + dx, y - 45 + i * 5, x + dx + e.facing * 24, y - 15 + i * 4, RGB(40, 25, 52), 2);
            }
        } else {
            for (int band = 0; band < bands; ++band) {
                std::vector<POINT> pts;
                for (int i = 0; i <= 14; ++i) {
                    const float u = static_cast<float>(i) / 14.f;
                    const float arc = -1.05f + u * 2.1f;
                    const float px = e.p.x + e.facing * (std::cos(arc) * r * 0.72f + 14.f * band);
                    const float py = e.p.y + std::sin(arc) * r * 0.42f + (u - 0.5f) * 28.f;
                    pts.push_back({static_cast<LONG>(px), static_cast<LONG>(py)});
                }
                stroke(hdc, pts, band == 0 ? RGB(255, 238, 247) : RGB(255, 145, 195), wide);
            }
            for (int i = 0; i < 5; ++i) {
                const float a = -0.9f + i * 0.45f;
                ellipse(hdc, x + e.facing * static_cast<int>(std::cos(a) * r * 0.55f), y + static_cast<int>(std::sin(a) * r * 0.42f), 14, 8, RGB(255, 180, 215), RGB(255, 238, 247), 1);
            }
        }
    }

    void drawRoleCharge(HDC hdc, const Effect& e, int x, int y, int r) {
        const int role = e.role < 0 ? 0 : e.role;
        const bool release = e.type == EffectType::ChargeRelease;
        if (!release) {
            ellipse(hdc, x - r, y - r, r * 2, r * 2, RGB(30, 36, 39), role == 1 ? RGB(112, 72, 150) : e.color, 1 + e.level * 2);
        }
        if (role == 0) {
            const int count = release ? 7 : 6 + e.level * 3;
            for (int i = 0; i < count; ++i) {
                const float a = i * 6.28318f / static_cast<float>(count) + (release ? 0.f : e.life * 5.f);
                const int x1 = x + static_cast<int>(std::cos(a) * r * (release ? 0.28f : 1.15f));
                const int y1 = y + static_cast<int>(std::sin(a) * r * (release ? 0.22f : 0.68f));
                const int x2 = x + e.facing * static_cast<int>(r * (release ? 1.0f : 0.55f)) + static_cast<int>(std::cos(a) * r * 0.12f);
                const int y2 = y + static_cast<int>(std::sin(a) * r * (release ? 0.45f : 0.34f));
                line(hdc, x1, y1, x2, y2, i % 2 ? RGB(115, 205, 255) : RGB(238, 252, 255), release ? 4 : 2 + e.level);
            }
        } else if (role == 1) {
            const int count = release ? 10 : 7 + e.level * 4;
            for (int i = 0; i < count; ++i) {
                const float a = i * 6.28318f / static_cast<float>(count) - e.life * 4.f;
                const int x1 = x + static_cast<int>(std::cos(a) * r * (release ? 0.9f : 1.25f));
                const int y1 = y + static_cast<int>(std::sin(a) * r * (release ? 0.62f : 0.72f));
                const int x2 = release ? x + e.facing * static_cast<int>(r * 0.25f) : x + static_cast<int>(std::cos(a) * r * 0.42f);
                const int y2 = release ? y + static_cast<int>(std::sin(a) * r * 0.18f) : y + static_cast<int>(std::sin(a) * r * 0.28f);
                line(hdc, x1, y1, x2, y2, i % 3 ? RGB(42, 27, 52) : RGB(165, 105, 230), release ? 5 : 2 + e.level);
            }
        } else {
            const int count = release ? 12 : 6 + e.level * 3;
            for (int i = 0; i < count; ++i) {
                const float a = i * 6.28318f / static_cast<float>(count) + e.life * 2.f;
                const int px = x + static_cast<int>(std::cos(a) * r * (release ? 0.72f : 1.0f));
                const int py = y + static_cast<int>(std::sin(a) * r * (release ? 0.48f : 0.6f));
                ellipse(hdc, px - 7, py - 4, 14, 8, i % 2 ? RGB(255, 145, 195) : RGB(255, 225, 240), RGB(255, 238, 247), 1);
                if (release) line(hdc, x, y, px, py, RGB(255, 180, 215), 2);
            }
        }
    }

    void drawRoleAura(HDC hdc, const Effect& e, int x, int y, int r, float t) {
        const int role = e.role < 0 ? 0 : e.role;
        const int rings = e.type == EffectType::UltimateAura ? 4 : 2;
        if (role == 0) {
            for (int i = 0; i < rings; ++i) {
                const int rr = static_cast<int>(r * (0.42f + i * 0.2f + (1.f - t) * 0.2f));
                ellipse(hdc, x - rr, y - rr, rr * 2, rr * 2, RGB(30, 36, 39), i == 0 ? RGB(238, 252, 255) : RGB(115, 205, 255), e.type == EffectType::UltimateAura ? 5 : 3);
            }
            for (int i = 0; i < 10; ++i) {
                const float a = i * 6.28318f / 10.f;
                line(hdc, x + static_cast<int>(std::cos(a) * r * 0.25f), y + static_cast<int>(std::sin(a) * r * 0.25f), x + static_cast<int>(std::cos(a) * r * 0.85f), y + static_cast<int>(std::sin(a) * r * 0.85f), RGB(180, 235, 255), 2);
            }
        } else if (role == 1) {
            for (int i = 0; i < rings; ++i) {
                const int rr = static_cast<int>(r * (0.35f + i * 0.2f + (1.f - t) * 0.25f));
                ellipse(hdc, x - rr, y - rr, rr * 2, rr * 2, RGB(30, 36, 39), i == 0 ? RGB(245, 245, 235) : RGB(165, 105, 230), e.type == EffectType::UltimateAura ? 5 : 3);
            }
            for (int i = -3; i <= 3; ++i) {
                line(hdc, x + i * 24, y - r, x + i * 12, y + r / 2, RGB(42, 27, 52), e.type == EffectType::UltimateAura ? 4 : 2);
            }
        } else {
            for (int i = 0; i < rings; ++i) {
                const int rr = static_cast<int>(r * (0.42f + i * 0.18f + (1.f - t) * 0.22f));
                ellipse(hdc, x - rr, y - rr, rr * 2, rr * 2, RGB(30, 36, 39), i == 0 ? RGB(255, 238, 247) : RGB(255, 145, 195), e.type == EffectType::UltimateAura ? 5 : 3);
            }
            for (int i = 0; i < 12; ++i) {
                const float a = i * 6.28318f / 12.f;
                const int px = x + static_cast<int>(std::cos(a) * r * 0.62f);
                const int py = y + static_cast<int>(std::sin(a) * r * 0.44f);
                ellipse(hdc, px - 10, py - 6, 20, 12, RGB(255, 180, 215), RGB(255, 238, 247), 1);
            }
        }
    }

    void drawEffects(HDC hdc, bool behind) {
        for (const auto& e : effects) {
            if (behind != (e.type == EffectType::DodgeTrail || e.type == EffectType::ChargeLoop)) continue;
            if (e.type == EffectType::UltimateAura &&
                e.role >= 0 &&
                e.role < static_cast<int>(characterUltimateVfx.size()) &&
                characterUltimateVfx[static_cast<size_t>(e.role)].bitmap) {
                continue;
            }
            const float t = clampf(e.life / e.maxLife, 0.f, 1.f);
            const int x = static_cast<int>(e.p.x);
            const int y = static_cast<int>(e.p.y);
            const int r = static_cast<int>(e.radius * (1.05f - t * 0.25f));
            if (e.type == EffectType::Slash) {
                drawRoleSlash(hdc, e, x, y, r);
            } else if (e.type == EffectType::ChargeLoop) {
                drawRoleCharge(hdc, e, x, y, r);
            } else if (e.type == EffectType::ChargeRelease) {
                drawRoleCharge(hdc, e, x, y, r);
            } else if (e.type == EffectType::ParryGuard) {
                POINT pts[4] = {
                    {x + e.facing * 22, y - 112},
                    {x + e.facing * 98, y - 76},
                    {x + e.facing * 98, y + 10},
                    {x + e.facing * 22, y + 42}
                };
                HPEN pen = CreatePen(PS_SOLID, 5, RGB(245, 245, 235));
                HGDIOBJ old = SelectObject(hdc, pen);
                Polyline(hdc, pts, 4);
                SelectObject(hdc, old);
                DeleteObject(pen);
            } else if (e.type == EffectType::ParrySuccess) {
                for (int i = 0; i < 3; ++i) {
                    int rr = static_cast<int>(r * (0.45f + i * 0.28f));
                    ellipse(hdc, x - rr, y - rr, rr * 2, rr * 2, RGB(30, 36, 39), RGB(245, 245, 235), 4 - i);
                }
                line(hdc, x - r, y, x + r, y, RGB(245, 245, 235), 4);
                line(hdc, x, y - r, x, y + r, RGB(245, 245, 235), 4);
            } else if (e.type == EffectType::ParryBreak) {
                for (int i = 0; i < 9; ++i) {
                    const float a = i * 6.28318f / 9.f;
                    line(hdc, x, y - 38, x + static_cast<int>(std::cos(a) * r), y - 38 + static_cast<int>(std::sin(a) * r * 0.7f), RGB(145, 145, 145), 3);
                }
            } else if (e.type == EffectType::DodgeTrail) {
                rect(hdc, x - 22, y - 82, 44, 88, RGB(88, 96, 96));
                ellipse(hdc, x - 18, y - 112, 36, 36, RGB(118, 126, 126), RGB(118, 126, 126));
                line(hdc, x, y - 25, x - e.facing * 95, y - 10, RGB(235, 235, 225), 3);
            } else if (e.type == EffectType::HitBurst) {
                for (int i = 0; i < 12; ++i) {
                    const float a = i * 6.28318f / 12.f;
                    line(hdc, x, y - 46, x + static_cast<int>(std::cos(a) * r), y - 46 + static_cast<int>(std::sin(a) * r), e.color, e.level == 2 ? 5 : 3);
                }
            } else if (e.type == EffectType::SkillAura || e.type == EffectType::UltimateAura) {
                drawRoleAura(hdc, e, x, y, r, t);
            }
        }
    }

    bool drawUltimateVfxSprite(HDC hdc, const Fighter& f, int x, int y) {
        if (f.action != Action::Ultimate) return false;
        const int role = characterIndex(f);
        const BitmapImage& image = characterUltimateVfx[static_cast<size_t>(role)];
        if (!image.bitmap || image.width <= 0 || image.height <= 0) return false;
        const int drawW = role == 1 ? 650 : 670;
        const int drawH = std::max(220, std::min(380, static_cast<int>(drawW * static_cast<float>(image.height) / static_cast<float>(image.width))));
        const int drawX = x - drawW / 2 + f.facing * 44;
        const int drawY = y - drawH + 104;
        return drawBitmapAlpha(hdc, image, drawX, drawY, drawW, drawH, f.facing > 0, 220);
    }

    bool drawWeaponSprite(HDC hdc, const Fighter& f, int x, int y, int weapon) {
        const BitmapImage& image = weaponModelSprites[static_cast<size_t>(std::max(0, std::min(weapon, 2)))];
        if (!image.bitmap || image.width <= 0 || image.height <= 0) return false;
        const int d = f.facing;
        const int handX = x + d * 22;
        const int handY = y - 55;
        int drawW = 132;
        int drawH = std::max(24, static_cast<int>(drawW * static_cast<float>(image.height) / static_cast<float>(image.width)));
        int drawX = d > 0 ? handX - 18 : handX - drawW + 18;
        int drawY = handY - drawH / 2 - 2;
        BYTE alpha = f.action == Action::Hit ? 180 : 255;

        if (weapon == 1) {
            drawW = 152;
            drawH = std::max(38, static_cast<int>(drawW * static_cast<float>(image.height) / static_cast<float>(image.width)));
            drawX = d > 0 ? handX - 24 : handX - drawW + 24;
            drawY = handY - drawH / 2 + 2;
        } else if (weapon == 2) {
            drawW = 118;
            drawH = std::max(74, static_cast<int>(drawW * static_cast<float>(image.height) / static_cast<float>(image.width)));
            drawX = handX - drawW / 2 + d * 42;
            drawY = handY - drawH / 2 - 2;
        }

        return drawBitmapAlpha(hdc, image, drawX, drawY, drawW, drawH, d < 0, alpha);
    }

    void drawWeapon(HDC hdc, const Fighter& f, int x, int y, int role, int weapon) {
        if (drawWeaponSprite(hdc, f, x, y, weapon)) return;
        const int d = f.facing;
        const int handX = x + d * 22;
        const int handY = y - 55;
        const COLORREF handle = role == 1 ? RGB(45, 32, 55) : RGB(70, 58, 42);
        const COLORREF blade = f.action == Action::Hit ? RGB(245, 245, 245) : RGB(222, 228, 225);
        if (weapon == 0) {
            line(hdc, handX, handY, handX + d * 96, handY - 14, blade, 5);
            line(hdc, handX + d * 18, handY - 2, handX + d * 102, handY - 14, RGB(245, 245, 235), 2);
            line(hdc, handX - d * 8, handY + 5, handX + d * 17, handY - 8, handle, 5);
            line(hdc, handX + d * 12, handY + 8, handX + d * 19, handY - 12, RGB(95, 76, 45), 3);
        } else if (weapon == 1) {
            std::vector<POINT> bladePts = {
                {handX + d * 18, handY - 10},
                {handX + d * 108, handY - 38},
                {handX + d * 122, handY - 18},
                {handX + d * 88, handY + 9},
                {handX + d * 22, handY + 6}
            };
            fillPoly(hdc, bladePts, blade, RGB(90, 92, 88), 2);
            line(hdc, handX - d * 14, handY + 15, handX + d * 30, handY - 7, handle, 8);
            line(hdc, handX + d * 28, handY - 10, handX + d * 44, handY + 15, RGB(120, 88, 45), 5);
        } else {
            line(hdc, handX - d * 12, handY - 6, handX + d * 58, handY - 30, blade, 5);
            line(hdc, handX + d * 4, handY + 20, handX + d * 70, handY + 2, blade, 5);
            line(hdc, handX - d * 18, handY - 1, handX + d * 16, handY - 11, handle, 5);
            line(hdc, handX - d * 10, handY + 24, handX + d * 22, handY + 15, handle, 5);
            line(hdc, handX + d * 42, handY - 28, handX + d * 64, handY - 31, RGB(245, 245, 235), 2);
            line(hdc, handX + d * 48, handY + 4, handX + d * 76, handY + 0, RGB(245, 245, 235), 2);
        }
    }

    void drawHitFlash(HDC hdc, const Fighter& f, int x, int y) {
        if (f.flashTimer <= 0.f) return;
        const int pulse = static_cast<int>(f.flashTimer * 35.f);
        HPEN pen = CreatePen(PS_SOLID, 3, RGB(245, 245, 245));
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        HGDIOBJ oldBrush = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
        Ellipse(hdc, x - 42 - pulse, y - 126 - pulse, x + 42 + pulse, y + 34 + pulse);
        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
        line(hdc, x - 48, y - 86, x + 48, y - 34, RGB(245, 245, 245), 4);
        line(hdc, x + 48, y - 86, x - 48, y - 34, RGB(245, 245, 245), 4);
    }

    void drawFighter(HDC hdc, const Fighter& f) {
        int x = static_cast<int>(f.p.x);
        int y = static_cast<int>(f.p.y - f.z);
        const int role = characterIndex(f);
        const int weapon = weaponIndex(f);
        const COLORREF hitBody = f.action == Action::Hit ? RGB(245, 245, 245) : f.c.color;
        ellipse(hdc, x - 48, static_cast<int>(f.p.y) + 20, 96, 24, RGB(10, 10, 10), RGB(10, 10, 10));
        drawUltimateVfxSprite(hdc, f, x, y);
        if ((
                f.action == Action::Idle ||
                f.action == Action::Normal ||
                f.action == Action::Charging ||
                f.action == Action::ChargeRelease ||
                f.action == Action::Dodge ||
                f.action == Action::Parry ||
                f.action == Action::Skill ||
                f.action == Action::Ultimate ||
                f.action == Action::Hit ||
                f.airborne) &&
            drawCharacterSprite(hdc, f, x, y)) {
            drawWeapon(hdc, f, x, y, role, weapon);
            drawHitFlash(hdc, f, x, y);
            return;
        }
        if (role == 0) {
            line(hdc, x - f.facing * 8, y - 105, x - f.facing * 42, y - 60, RGB(238, 252, 255), 5);
            std::vector<POINT> coat = {
                {x - 22, y - 84},
                {x + 24, y - 80},
                {x + 18, y + 16},
                {x - 18, y + 18}
            };
            fillPoly(hdc, coat, hitBody, RGB(35, 80, 98), 2);
            rect(hdc, x - 18, y - 42, 36, 8, RGB(238, 252, 255));
            line(hdc, x - 28, y - 76, x - 42, y - 10, RGB(55, 86, 104), 5);
            line(hdc, x + 28, y - 76, x + 40, y - 12, RGB(55, 86, 104), 5);
            line(hdc, x - f.facing * 4, y - 112, x - f.facing * 12, y - 58, RGB(238, 252, 255), 4);
        } else if (role == 1) {
            std::vector<POINT> robe = {
                {x - 34, y - 84},
                {x + 34, y - 82},
                {x + 42, y + 18},
                {x + 9, y + 28},
                {x - 42, y + 18}
            };
            fillPoly(hdc, robe, f.action == Action::Hit ? RGB(225, 218, 230) : RGB(42, 31, 54), RGB(165, 105, 230), 2);
            rect(hdc, x - 20, y - 46, 40, 9, RGB(165, 105, 230));
            line(hdc, x - 34, y - 72, x - 70, y - 28, RGB(30, 24, 38), 9);
            line(hdc, x + 34, y - 72, x + 70, y - 28, RGB(30, 24, 38), 9);
            rect(hdc, x - 30, y - 123, 60, 9, RGB(30, 24, 38));
        } else {
            std::vector<POINT> dress = {
                {x - 24, y - 84},
                {x + 24, y - 84},
                {x + 44, y + 20},
                {x, y + 32},
                {x - 44, y + 20}
            };
            fillPoly(hdc, dress, f.action == Action::Hit ? RGB(255, 238, 247) : RGB(255, 210, 230), RGB(255, 145, 195), 2);
            rect(hdc, x - 18, y - 76, 36, 42, RGB(248, 238, 232));
            rect(hdc, x - 24, y - 41, 48, 8, RGB(255, 145, 195));
            line(hdc, x - 24, y - 70, x - 52, y - 22, RGB(255, 180, 215), 6);
            line(hdc, x + 24, y - 70, x + 52, y - 22, RGB(255, 180, 215), 6);
        }
        ellipse(hdc, x - 21, y - 115, 42, 42, RGB(232, 222, 205));
        drawHitFlash(hdc, f, x, y);
        if (role == 0) {
            line(hdc, x - 13, y - 114, x - 24, y - 70, RGB(238, 252, 255), 5);
            line(hdc, x + 10, y - 114, x + 3, y - 72, RGB(205, 235, 245), 3);
        } else if (role == 1) {
            rect(hdc, x - 20, y - 116, 40, 13, RGB(32, 24, 38));
            line(hdc, x - 16, y - 96, x + 16, y - 96, RGB(165, 105, 230), 2);
        } else {
            ellipse(hdc, x + 7, y - 123, 12, 8, RGB(255, 145, 195), RGB(255, 238, 247), 1);
            ellipse(hdc, x + 15, y - 118, 12, 8, RGB(255, 180, 215), RGB(255, 238, 247), 1);
        }
        drawWeapon(hdc, f, x, y, role, weapon);
        if (f.action == Action::Charging) {
            const COLORREF glow = role == 0 ? RGB(180, 235, 255) : (role == 1 ? RGB(92, 55, 130) : RGB(255, 180, 215));
            rect(hdc, x + f.facing * 26, y - 68, f.facing * 42, 12, glow);
        }
        if (f.action == Action::Parry) {
            line(hdc, x + f.facing * 18, y - 92, x + f.facing * 18, y - 12, RGB(245, 245, 235), 6);
        }
    }

    ActionSpriteSlot actionSpriteFor(const Fighter& f, bool moving) const {
        ActionSpriteSlot slot = ActionSpriteSlot::Idle;
        if (f.action == Action::Hit) {
            slot = ActionSpriteSlot::Hit;
        } else if (f.action == Action::Ultimate) {
            slot = ActionSpriteSlot::Ultimate;
        } else if (f.action == Action::Skill) {
            slot = ActionSpriteSlot::Skill;
        } else if (f.action == Action::ChargeRelease) {
            slot = ActionSpriteSlot::ChargeRelease;
        } else if (f.action == Action::Charging) {
            slot = ActionSpriteSlot::Charging;
        } else if (f.action == Action::Normal) {
            slot = ActionSpriteSlot::Normal;
        } else if (f.action == Action::Parry) {
            slot = ActionSpriteSlot::Parry;
        } else if (f.action == Action::Dodge) {
            slot = ActionSpriteSlot::Dodge;
        } else if (f.airborne) {
            slot = ActionSpriteSlot::Jump;
        } else if (moving) {
            slot = (static_cast<int>(animationClock * 8.f) % 2 == 0) ? ActionSpriteSlot::Run1 : ActionSpriteSlot::Run2;
        }
        return slot;
    }

    bool drawCharacterSprite(HDC hdc, const Fighter& f, int x, int y) {
        const bool playerSprite = &f == &player;
        const Vec move = playerSprite ? input.move : aiInput.move;
        const bool moving = absf(move.x) > 0.1f || absf(move.y) > 0.1f;
        const int role = characterIndex(f);
        ActionSpriteSlot slot = actionSpriteFor(f, moving);
        if (!characterSprite(role, slot).bitmap && slot == ActionSpriteSlot::Run2) slot = ActionSpriteSlot::Run1;
        const BitmapImage* image = &characterSprite(role, slot);
        if (!image->bitmap) image = &characterSprite(role, ActionSpriteSlot::Idle);
        if (!image->bitmap) return false;
        const ActionSpriteLayout layout = spriteLayoutFor(slot, *image, role);
        const int drawW = layout.width;
        const int drawH = layout.height;
        const int drawX = x - drawW / 2 + f.facing * layout.xOffset;
        const int drawY = y - drawH + layout.yOffset;
        const bool mirrorSprite = layout.mirrorFromSource ? f.facing > 0 : f.facing < 0;
        return drawBitmapAlpha(hdc, *image, drawX, drawY, drawW, drawH, mirrorSprite);
    }

    void bar(HDC hdc, int x, int y, int w, int h, float k, COLORREF c) {
        rect(hdc, x, y, w, h, RGB(18, 18, 18));
        rect(hdc, x, y, static_cast<int>(w * clampf(k, 0.f, 1.f)), h, c);
    }

    void drawHud(HDC hdc) {
        text(hdc, std::wstring(player.c.name) + L" / " + player.w.name, 24, 22, 18, RGB(240, 236, 220));
        bar(hdc, 24, 54, 300, 16, player.hp / player.c.hp, RGB(180, 45, 50));
        bar(hdc, 24, 76, 300, 10, player.energy / 100.f, RGB(70, 145, 230));
        bar(hdc, 24, 92, 300, 10, player.stamina / 100.f, RGB(220, 180, 70));
        text(hdc, std::wstring(ai.c.name) + L" / " + ai.w.name, W - 324, 22, 18, RGB(240, 236, 220));
        bar(hdc, W - 324, 54, 300, 16, ai.hp / ai.c.hp, RGB(180, 45, 50));
        bar(hdc, W - 324, 76, 300, 10, ai.energy / 100.f, RGB(70, 145, 230));
        bar(hdc, W - 324, 92, 300, 10, ai.stamina / 100.f, RGB(220, 180, 70));
        text(hdc, std::to_wstring(static_cast<int>(std::ceil(std::max(0.f, timeLeft)))), 0, 26, 34, RGB(235, 230, 215), true);
        if (messageTimer > 0.f) text(hdc, message, 0, 145, 48, RGB(245, 245, 235), true);
        drawBattleTips(hdc);
    }

    void drawMenu(HDC hdc) {
        if (screen == Screen::Main) {
            const std::vector<std::wstring> opts = {L"开始对战", L"练习模式", L"退出游戏"};
            for (int i = 0; i < static_cast<int>(opts.size()); ++i) {
                const int x = 480;
                const int y = 450 + i * 66;
                COLORREF bg = i == menu ? RGB(225, 222, 205) : RGB(30, 32, 32);
                COLORREF fg = i == menu ? RGB(25, 25, 25) : RGB(240, 235, 220);
                rect(hdc, x, y, 320, 48, bg);
                RECT label{x, y, x + 320, y + 48};
                textInRect(hdc, opts[i], label, 21, fg);
            }
            return;
        }

        text(hdc, L"墨 刃", 0, 92, 58, RGB(242, 238, 220), true);
        std::vector<std::wstring> opts;
        std::wstring sub;
        if (screen == Screen::Character) {
            sub = L"选择角色";
            for (auto& c : Characters) opts.push_back(std::wstring(c.name) + L" - " + c.title);
        } else if (screen == Screen::Weapon) {
            sub = L"选择武器";
            for (auto& w : Weapons) opts.push_back(std::wstring(w.name) + L" - " + w.style);
        } else if (screen == Screen::Practice) {
            drawPracticeSettings(hdc);
            return;
        } else {
            sub = L"选择 AI 难度";
            opts = {L"简单", L"普通", L"困难"};
        }
        text(hdc, sub, 0, 166, 22, RGB(190, 196, 185), true);
        for (int i = 0; i < static_cast<int>(opts.size()); ++i) {
            COLORREF bg = i == menu ? RGB(225, 222, 205) : RGB(40, 40, 38);
            COLORREF fg = i == menu ? RGB(25, 25, 25) : RGB(240, 235, 220);
            rect(hdc, 380, 260 + i * 70, 520, 48, bg);
            text(hdc, opts[i], 0, 272 + i * 70, 23, fg, true);
        }
        if (screen == Screen::Character) {
            text(hdc, Characters[menu].passive, 410, 520, 18, RGB(235, 230, 220));
            text(hdc, Characters[menu].skill, 410, 550, 18, RGB(235, 230, 220));
            text(hdc, Characters[menu].ult, 410, 580, 18, RGB(235, 230, 220));
        }
        if (screen == Screen::Weapon) {
            text(hdc, L"连招段数：" + std::to_wstring(Weapons[menu].combo.size()), 470, 520, 18, RGB(235, 230, 220));
            text(hdc, L"不同武器拥有不同振刀窗口和反击倍率", 470, 550, 18, RGB(235, 230, 220));
        }
    }

    void drawPracticeSettings(HDC hdc) {
        text(hdc, L"练习模式设置", 0, 142, 28, RGB(242, 238, 220), true);
        text(hdc, L"默认敌人静止，适合测试平A、蓄力、振刀、技能和大招。", 0, 186, 18, RGB(210, 214, 202), true);
        const std::vector<std::wstring> opts = {
            std::wstring(L"敌人攻击：") + (practiceEnemyAttacks ? L"开" : L"关"),
            std::wstring(L"玩家满大招能量：") + (practiceFullEnergy ? L"开" : L"关"),
            std::wstring(L"敌人满大招能量：") + (practiceEnemyFullEnergy ? L"开" : L"关"),
            L"开始练习"
        };
        for (int i = 0; i < static_cast<int>(opts.size()); ++i) {
            COLORREF bg = i == menu ? RGB(225, 222, 205) : RGB(40, 40, 38);
            COLORREF fg = i == menu ? RGB(25, 25, 25) : RGB(240, 235, 220);
            rect(hdc, 380, 260 + i * 70, 520, 48, bg);
            text(hdc, opts[i], 0, 272 + i * 70, 23, fg, true);
        }
        text(hdc, L"Enter 切换选项 / 开始，Esc 返回主菜单", 0, 568, 18, RGB(210, 214, 202), true);
    }

    void drawBattleTips(HDC hdc) {
        rect(hdc, 24, H - 122, 460, 96, RGB(32, 34, 32));
        text(hdc, L"操作提示", 42, H - 112, 19, RGB(242, 238, 220));
        text(hdc, L"WASD：移动    Space：跳跃    Shift：闪避", 42, H - 84, 16, RGB(220, 218, 205));
        text(hdc, L"鼠标左键：平A / 长按蓄力    G：振刀", 42, H - 60, 16, RGB(220, 218, 205));
        text(hdc, L"F：小技能    V：大招（能量满）    Esc：返回主菜单", 42, H - 36, 16, RGB(220, 218, 205));
    }

    void drawResult(HDC hdc) {
        rect(hdc, 0, 0, W, H, RGB(5, 5, 5));
        text(hdc, win ? L"胜 利" : L"失 败", 0, 170, 64, win ? RGB(235, 230, 205) : RGB(215, 80, 80), true);
        text(hdc, L"Enter：再来一局    Esc：返回菜单", 0, 250, 24, RGB(230, 226, 210), true);
        text(hdc, L"总伤害：" + std::to_wstring(static_cast<int>(player.damageDone)), 500, 330, 22, RGB(235, 230, 220));
        text(hdc, L"振刀成功：" + std::to_wstring(player.parries), 500, 365, 22, RGB(235, 230, 220));
        text(hdc, L"最大连招：" + std::to_wstring(player.maxCombo), 500, 400, 22, RGB(235, 230, 220));
        text(hdc, L"蓄力命中：" + std::to_wstring(player.chargeHits), 500, 435, 22, RGB(235, 230, 220));
        text(hdc, L"闪避次数：" + std::to_wstring(player.dodges), 500, 470, 22, RGB(235, 230, 220));
        rect(hdc, 420, 520, 200, 52, RGB(225, 222, 205));
        rect(hdc, 660, 520, 200, 52, RGB(40, 40, 38));
        text(hdc, L"再来一局", 470, 532, 22, RGB(25, 25, 25), false);
        text(hdc, L"返回菜单", 710, 532, 22, RGB(240, 235, 220), false);
    }
};

Game g;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            g.init(hwnd);
            SetTimer(hwnd, 1, 16, nullptr);
            return 0;
        case WM_TIMER:
            g.tick(1.f / 60.f);
            return 0;
        case WM_KEYDOWN:
            g.onKey(wParam);
            return 0;
        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            g.onMouse(true, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_LBUTTONUP:
            ReleaseCapture();
            g.onMouse(false, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            g.onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            g.paint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    if (std::wstring(GetCommandLineW()).find(L"--self-test-death") != std::wstring::npos) {
        return g.runDeathSelfTest() ? 0 : 2;
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"InkBladeWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    RECT rc{0, 0, W, H};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"墨刃 MVP", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top,
                                nullptr, nullptr, hInstance, nullptr);
    if (!hwnd) return 1;
    ShowWindow(hwnd, nCmdShow);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

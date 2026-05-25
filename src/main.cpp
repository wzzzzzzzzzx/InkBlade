#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr float WorldW = 1280.f;
constexpr float WorldH = 720.f;
constexpr float BaseDamage = 100.f;

float clampf(float value, float low, float high) {
    return std::max(low, std::min(value, high));
}

float length(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalized(sf::Vector2f v) {
    const float len = length(v);
    if (len < 0.001f) {
        return {0.f, 0.f};
    }
    return {v.x / len, v.y / len};
}

sf::Color withAlpha(sf::Color color, sf::Uint8 alpha) {
    color.a = alpha;
    return color;
}

struct CharacterDef {
    std::string id;
    std::string name;
    std::string title;
    float hp;
    float attack;
    float speed;
    float staminaRegen;
    float hitEnergyMod;
    float takenEnergyMod;
    sf::Color color;
    std::string passive;
    std::string skill;
    std::string ultimate;
};

struct WeaponDef {
    std::string id;
    std::string name;
    std::string style;
    std::vector<float> combo;
    std::array<float, 3> charge;
    float parryWindow;
    float counter;
};

const std::vector<CharacterDef> Characters = {
    {"lingshuang", "Ling Shuang", "Frost Duelist", 900.f, 1.15f, 260.f, 15.f, 1.25f, 1.f, {115, 205, 255}, "Damage +15%", "Frost Dash: dash and slow", "Ice Bloom: armored slashes"},
    {"mohen", "Mo Hen", "Ink Binder", 1000.f, 0.9f, 280.f, 15.f, 1.f, 1.f, {165, 105, 230}, "Cooldowns -20%", "Ink Seal: bind target", "Ink Net: no dodge zone"},
    {"suxin", "Su Xin", "Lotus Healer", 1100.f, 0.85f, 235.f, 22.f, 1.f, 1.5f, {255, 145, 195}, "Heals 5% damage taken", "Spring Heal: restore HP", "Lotus Mercy: regen + reduction"}
};

const std::vector<WeaponDef> Weapons = {
    {"longsword", "Longsword", "Balanced", {1.f, 1.f, 1.5f}, {1.5f, 2.f, 3.f}, 0.17f, 1.2f},
    {"broadsword", "Broadsword", "Heavy", {1.3f, 2.f}, {2.f, 2.8f, 4.f}, 0.13f, 2.f},
    {"dualblade", "Dual Blade", "Rapid", {0.6f, 0.6f, 0.8f, 0.8f, 1.2f}, {1.5f, 2.2f, 2.8f}, 0.2f, 0.8f}
};

enum class Screen {
    MainMenu,
    CharacterSelect,
    WeaponSelect,
    DifficultySelect,
    Battle,
    Result
};

enum class Action {
    Idle,
    Normal,
    Charging,
    ChargeRelease,
    Parry,
    Counter,
    Dodge,
    Skill,
    Ultimate,
    Hit,
    Down
};

enum class Difficulty {
    Easy,
    Normal,
    Hard
};

struct InputFrame {
    bool attackPressed = false;
    bool attackReleased = false;
    bool attackHeld = false;
    bool parry = false;
    bool dodge = false;
    bool skill = false;
    bool ultimate = false;
    bool jump = false;
    sf::Vector2f move {};
};

struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color color;
    float life;
    float maxLife;
    float radius;
};

struct Fighter {
    CharacterDef character;
    WeaponDef weapon;
    sf::Vector2f pos;
    sf::Vector2f vel;
    int facing = 1;
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
    int comboIndex = 0;
    bool airborne = false;
    float z = 0.f;
    float vz = 0.f;
    Action action = Action::Idle;
    float damageDone = 0.f;
    int parries = 0;
    int maxCombo = 0;
    int chargeHits = 0;
    int dodges = 0;
};

struct Button {
    std::string text;
};

class Game {
public:
    Game()
        : window(sf::VideoMode(static_cast<unsigned>(WorldW), static_cast<unsigned>(WorldH)), "Ink Blade MVP"),
          rng(std::random_device{}()) {
        window.setFramerateLimit(60);
        resetBattle();
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            const float dt = std::min(clock.restart().asSeconds(), 0.033f);
            handleEvents();
            update(dt);
            render();
        }
    }

private:
    sf::RenderWindow window;
    Screen screen = Screen::MainMenu;
    Difficulty difficulty = Difficulty::Normal;
    std::mt19937 rng;
    std::vector<Particle> particles;
    Fighter player;
    Fighter ai;
    int menuIndex = 0;
    int selectedCharacter = 0;
    int selectedWeapon = 0;
    int selectedDifficulty = 1;
    float matchTime = 99.f;
    float aiThinkTimer = 0.f;
    InputFrame aiInput;
    InputFrame input;
    bool mouseWasDown = false;
    bool showWin = false;
    float messageTimer = 0.f;
    std::string message;

    void resetBattle() {
        player = makeFighter(Characters[selectedCharacter], Weapons[selectedWeapon], {260.f, 440.f}, 1);
        const int aiChar = (selectedCharacter + 1) % static_cast<int>(Characters.size());
        const int aiWeapon = (selectedWeapon + 1) % static_cast<int>(Weapons.size());
        ai = makeFighter(Characters[aiChar], Weapons[aiWeapon], {1020.f, 440.f}, -1);
        matchTime = 99.f;
        aiThinkTimer = 0.f;
        particles.clear();
        message.clear();
        messageTimer = 0.f;
    }

    Fighter makeFighter(const CharacterDef& c, const WeaponDef& w, sf::Vector2f pos, int facing) {
        Fighter f;
        f.character = c;
        f.weapon = w;
        f.pos = pos;
        f.facing = facing;
        f.hp = c.hp;
        return f;
    }

    void handleEvents() {
        input = {};
        sf::Event event {};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                handleKeyPressed(event.key.code);
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                input.attackPressed = true;
            }
            if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
                input.attackReleased = true;
            }
        }

        input.attackHeld = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) input.move.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) input.move.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) input.move.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) input.move.y += 1.f;
        input.move = normalized(input.move);
        input.parry = sf::Keyboard::isKeyPressed(sf::Keyboard::G);
        input.dodge = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
        input.skill = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
        input.ultimate = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
        input.jump = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);

        const bool mouseDown = input.attackHeld;
        input.attackPressed = input.attackPressed || (mouseDown && !mouseWasDown);
        input.attackReleased = input.attackReleased || (!mouseDown && mouseWasDown);
        mouseWasDown = mouseDown;
    }

    void handleKeyPressed(sf::Keyboard::Key key) {
        if (screen == Screen::Battle) {
            if (key == sf::Keyboard::Escape) {
                screen = Screen::MainMenu;
                menuIndex = 0;
            }
            return;
        }

        if (screen == Screen::Result) {
            if (key == sf::Keyboard::Enter) {
                resetBattle();
                screen = Screen::Battle;
            } else if (key == sf::Keyboard::Escape) {
                screen = Screen::MainMenu;
                menuIndex = 0;
            }
            return;
        }

        const int optionCount = currentOptionCount();
        if (key == sf::Keyboard::Up || key == sf::Keyboard::W || key == sf::Keyboard::Left || key == sf::Keyboard::A) {
            menuIndex = (menuIndex + optionCount - 1) % optionCount;
            syncSelectionFromMenu();
        }
        if (key == sf::Keyboard::Down || key == sf::Keyboard::S || key == sf::Keyboard::Right || key == sf::Keyboard::D) {
            menuIndex = (menuIndex + 1) % optionCount;
            syncSelectionFromMenu();
        }
        if (key == sf::Keyboard::Enter) {
            confirmMenu();
        }
        if (key == sf::Keyboard::Escape) {
            backMenu();
        }
    }

    int currentOptionCount() const {
        switch (screen) {
            case Screen::MainMenu: return 3;
            case Screen::CharacterSelect: return static_cast<int>(Characters.size());
            case Screen::WeaponSelect: return static_cast<int>(Weapons.size());
            case Screen::DifficultySelect: return 3;
            default: return 1;
        }
    }

    void syncSelectionFromMenu() {
        if (screen == Screen::CharacterSelect) selectedCharacter = menuIndex;
        if (screen == Screen::WeaponSelect) selectedWeapon = menuIndex;
        if (screen == Screen::DifficultySelect) selectedDifficulty = menuIndex;
    }

    void confirmMenu() {
        if (screen == Screen::MainMenu) {
            if (menuIndex == 0) {
                screen = Screen::CharacterSelect;
                menuIndex = selectedCharacter;
            } else if (menuIndex == 1) {
                selectedDifficulty = 0;
                resetBattle();
                screen = Screen::Battle;
            } else {
                window.close();
            }
        } else if (screen == Screen::CharacterSelect) {
            selectedCharacter = menuIndex;
            screen = Screen::WeaponSelect;
            menuIndex = selectedWeapon;
        } else if (screen == Screen::WeaponSelect) {
            selectedWeapon = menuIndex;
            screen = Screen::DifficultySelect;
            menuIndex = selectedDifficulty;
        } else if (screen == Screen::DifficultySelect) {
            selectedDifficulty = menuIndex;
            difficulty = static_cast<Difficulty>(selectedDifficulty);
            resetBattle();
            screen = Screen::Battle;
        }
    }

    void backMenu() {
        if (screen == Screen::CharacterSelect) {
            screen = Screen::MainMenu;
            menuIndex = 0;
        } else if (screen == Screen::WeaponSelect) {
            screen = Screen::CharacterSelect;
            menuIndex = selectedCharacter;
        } else if (screen == Screen::DifficultySelect) {
            screen = Screen::WeaponSelect;
            menuIndex = selectedWeapon;
        }
    }

    void update(float dt) {
        updateParticles(dt);
        if (messageTimer > 0.f) {
            messageTimer -= dt;
        }
        if (screen != Screen::Battle) {
            spawnAmbient(dt);
            return;
        }

        matchTime -= dt;
        updateAi(dt);
        updateFighter(player, input, dt);
        updateFighter(ai, aiInput, dt);
        resolveCombat(player, ai);
        resolveCombat(ai, player);
        if (player.hp <= 0.f || ai.hp <= 0.f || matchTime <= 0.f) {
            showWin = player.hp >= ai.hp;
            screen = Screen::Result;
        }
    }

    void updateFighter(Fighter& f, const InputFrame& in, float dt) {
        f.cooldown = std::max(0.f, f.cooldown - dt);
        f.actionTimer = std::max(0.f, f.actionTimer - dt);
        f.hitTimer = std::max(0.f, f.hitTimer - dt);
        f.comboTimer = std::max(0.f, f.comboTimer - dt);
        f.dodgeLock = std::max(0.f, f.dodgeLock - dt);
        f.bindTimer = std::max(0.f, f.bindTimer - dt);
        f.slowTimer = std::max(0.f, f.slowTimer - dt);
        f.lotusTimer = std::max(0.f, f.lotusTimer - dt);

        if (f.actionTimer <= 0.f && f.action != Action::Charging && f.action != Action::Down) {
            f.action = Action::Idle;
        }

        if (f.hp <= 0.f) {
            f.action = Action::Down;
            return;
        }

        f.energy = clampf(f.energy + 2.f * dt, 0.f, 100.f);
        f.stamina = clampf(f.stamina + f.character.staminaRegen * dt, 0.f, 100.f);
        if (f.lotusTimer > 0.f && f.character.id == "suxin") {
            f.hp = clampf(f.hp + f.character.hp * 0.4f / 3.f * dt, 0.f, f.character.hp);
        }

        if (f.airborne) {
            f.vz -= 700.f * dt;
            f.z += f.vz * dt;
            if (f.z <= 0.f) {
                f.z = 0.f;
                f.vz = 0.f;
                f.airborne = false;
            }
        } else if (in.jump && isFree(f)) {
            f.airborne = true;
            f.vz = 410.f;
        }

        if (in.parry && isFree(f)) {
            f.action = Action::Parry;
            f.actionTimer = f.weapon.parryWindow;
            splash(f.pos, f.character.color, 12);
        } else if (in.dodge && isFree(f) && f.stamina >= 25.f && f.dodgeLock <= 0.f && f.bindTimer <= 0.f) {
            sf::Vector2f dir = in.move;
            if (length(dir) < 0.1f) {
                dir = {static_cast<float>(-f.facing), 0.f};
            }
            f.pos += normalized(dir) * 120.f;
            f.stamina -= 25.f;
            f.dodgeLock = 0.5f;
            f.action = Action::Dodge;
            f.actionTimer = 0.18f;
            f.dodges++;
            splash(f.pos, {245, 245, 235}, 16);
        } else if (in.skill && (isFree(f) || f.airborne) && f.cooldown <= 0.f) {
            useSkill(f);
        } else if (in.ultimate && (isFree(f) || f.airborne) && f.energy >= 100.f) {
            useUltimate(f);
        } else if (in.attackPressed && isFree(f)) {
            f.action = Action::Charging;
            f.chargeTimer = 0.f;
        } else if (in.attackHeld && f.action == Action::Charging) {
            f.chargeTimer += dt;
        } else if (in.attackReleased && f.action == Action::Charging) {
            f.action = Action::ChargeRelease;
            f.actionTimer = 0.28f;
            splash(f.pos + sf::Vector2f(static_cast<float>(f.facing) * 55.f, -15.f), {20, 20, 20}, 16);
        } else if (in.attackPressed && f.comboTimer > 0.f) {
            startNormal(f);
        }

        if (isFree(f) && f.bindTimer <= 0.f) {
            const float slow = f.slowTimer > 0.f ? 0.7f : 1.f;
            f.pos += in.move * f.character.speed * slow * dt;
        }

        if (std::abs(in.move.x) > 0.1f) {
            f.facing = in.move.x > 0.f ? 1 : -1;
        }

        f.pos.x = clampf(f.pos.x, 60.f, WorldW - 60.f);
        f.pos.y = clampf(f.pos.y, 250.f, WorldH - 80.f);
    }

    bool isFree(const Fighter& f) const {
        return f.action == Action::Idle || f.action == Action::Dodge;
    }

    void startNormal(Fighter& f) {
        f.action = Action::Normal;
        f.actionTimer = 0.18f + 0.04f * static_cast<float>(f.comboIndex);
        f.comboTimer = 0.32f;
        f.comboIndex = (f.comboIndex + 1) % static_cast<int>(f.weapon.combo.size());
        f.maxCombo = std::max(f.maxCombo, f.comboIndex + 1);
        splash(f.pos + sf::Vector2f(static_cast<float>(f.facing) * 50.f, -15.f), f.character.color, 10);
    }

    void useSkill(Fighter& f) {
        f.action = Action::Skill;
        f.actionTimer = 0.3f;
        f.cooldown = f.character.id == "mohen" ? 4.8f : (f.character.id == "lingshuang" ? 8.f : 10.f);
        if (f.character.id == "lingshuang") {
            f.pos.x += static_cast<float>(f.facing) * 170.f;
        } else if (f.character.id == "suxin") {
            f.hp = clampf(f.hp + f.character.hp * 0.2f, 0.f, f.character.hp);
        }
        splash(f.pos, f.character.color, 28);
    }

    void useUltimate(Fighter& f) {
        f.action = Action::Ultimate;
        f.actionTimer = 0.75f;
        f.energy = 0.f;
        if (f.character.id == "suxin") {
            f.lotusTimer = 3.f;
        }
        showMessage(f.character.name + " ultimate");
        splash(f.pos, f.character.color, 48);
    }

    void resolveCombat(Fighter& atk, Fighter& def) {
        if (atk.actionTimer <= 0.f) {
            return;
        }

        float range = 0.f;
        float mult = 0.f;
        bool charge = false;
        bool control = false;

        if (atk.action == Action::Normal) {
            range = 95.f;
            const int idx = std::max(0, (atk.comboIndex - 1 + static_cast<int>(atk.weapon.combo.size())) % static_cast<int>(atk.weapon.combo.size()));
            mult = atk.weapon.combo[static_cast<std::size_t>(idx)];
        } else if (atk.action == Action::ChargeRelease) {
            range = atk.weapon.id == "longsword" ? 360.f : (atk.weapon.id == "dualblade" ? 230.f : 130.f);
            const int lvl = chargeLevel(atk.chargeTimer);
            mult = atk.weapon.charge[static_cast<std::size_t>(lvl)] * (lvl == 0 ? 1.f : (lvl == 1 ? 1.2f : 1.5f));
            charge = true;
        } else if (atk.action == Action::Counter) {
            range = 120.f;
            mult = atk.weapon.counter;
        } else if (atk.action == Action::Skill) {
            range = atk.character.id == "mohen" ? 210.f : 105.f;
            mult = atk.character.id == "suxin" ? 0.f : 0.9f;
            control = true;
        } else if (atk.action == Action::Ultimate) {
            range = atk.character.id == "mohen" ? 390.f : 150.f;
            mult = atk.character.id == "lingshuang" ? 3.5f : (atk.character.id == "mohen" ? 1.5f : 0.f);
            control = true;
        } else {
            return;
        }

        const sf::Vector2f delta = def.pos - atk.pos;
        const bool inFront = atk.facing > 0 ? delta.x > -30.f : delta.x < 30.f;
        const bool inRange = std::abs(delta.x) < range && std::abs(delta.y) < 90.f && inFront;
        if (!inRange || def.hitTimer > 0.f || def.action == Action::Down) {
            return;
        }

        if (charge && def.action == Action::Parry && def.actionTimer > 0.f) {
            atk.action = Action::Hit;
            atk.actionTimer = 0.35f;
            def.action = Action::Counter;
            def.actionTimer = 0.28f;
            def.parries++;
            showMessage("PARRY");
            splash(def.pos, {250, 250, 250}, 34);
            return;
        }

        if (atk.action == Action::Normal && def.action == Action::Charging) {
            def.action = Action::Hit;
            def.actionTimer = 0.4f;
        }

        if (atk.action == Action::Normal && def.action == Action::Parry) {
            def.action = Action::Hit;
            def.actionTimer = 0.25f;
        }

        float damage = BaseDamage * mult * atk.character.attack;
        if (def.lotusTimer > 0.f) {
            damage *= 0.7f;
        }
        if (def.character.id == "suxin") {
            damage *= 0.95f;
        }

        def.hp = clampf(def.hp - damage, 0.f, def.character.hp);
        atk.damageDone += damage;
        atk.energy = clampf(atk.energy + 8.f * atk.character.hitEnergyMod, 0.f, 100.f);
        def.energy = clampf(def.energy + 5.f * def.character.takenEnergyMod, 0.f, 100.f);
        if (def.character.id == "suxin") {
            def.hp = clampf(def.hp + damage * 0.05f, 0.f, def.character.hp);
        }
        def.hitTimer = 0.25f;
        if (def.action != Action::Dodge) {
            def.action = Action::Hit;
            def.actionTimer = 0.25f;
        }
        def.pos.x += static_cast<float>(atk.facing) * (charge ? 45.f : 18.f);
        if (charge) {
            atk.chargeHits++;
        }
        if (control) {
            if (atk.character.id == "lingshuang") def.slowTimer = 2.f;
            if (atk.character.id == "mohen") def.bindTimer = atk.action == Action::Ultimate ? 2.f : 1.f;
        }
        splash(def.pos, atk.character.color, 22);
    }

    int chargeLevel(float t) const {
        if (t >= 1.f) return 2;
        if (t >= 0.5f) return 1;
        return 0;
    }

    void updateAi(float dt) {
        aiThinkTimer -= dt;
        aiInput = {};
        const float dist = std::abs(player.pos.x - ai.pos.x);
        const float react = difficulty == Difficulty::Easy ? 0.8f : (difficulty == Difficulty::Normal ? 0.45f : 0.18f);
        if (aiThinkTimer <= 0.f) {
            aiThinkTimer = react + random01() * react * 0.5f;
            const float parryChance = difficulty == Difficulty::Easy ? 0.1f : (difficulty == Difficulty::Normal ? 0.35f : 0.7f);
            const float dodgeChance = difficulty == Difficulty::Easy ? 0.1f : (difficulty == Difficulty::Normal ? 0.4f : 0.8f);
            const float skillChance = difficulty == Difficulty::Easy ? 0.3f : (difficulty == Difficulty::Normal ? 0.7f : 0.95f);
            if (player.action == Action::ChargeRelease && random01() < parryChance) {
                aiInput.parry = true;
            } else if (dist < 140.f && player.action == Action::Normal && random01() < dodgeChance && ai.stamina >= 25.f) {
                aiInput.dodge = true;
            } else if (ai.energy >= 100.f && random01() < skillChance) {
                aiInput.ultimate = true;
            } else if (ai.cooldown <= 0.f && random01() < skillChance * 0.35f) {
                aiInput.skill = true;
            } else if (dist < 150.f) {
                if (random01() < (difficulty == Difficulty::Easy ? 0.2f : 0.45f)) {
                    aiInput.attackPressed = true;
                } else {
                    ai.action = Action::Charging;
                    ai.chargeTimer = difficulty == Difficulty::Hard ? 1.1f : 0.55f;
                    aiInput.attackReleased = true;
                }
            }
        }

        if (dist > 130.f) {
            aiInput.move.x = player.pos.x > ai.pos.x ? 1.f : -1.f;
        } else if (dist < 90.f) {
            aiInput.move.x = player.pos.x > ai.pos.x ? -1.f : 1.f;
        }
        aiInput.move.y = clampf((player.pos.y - ai.pos.y) / 120.f, -1.f, 1.f);
        aiInput.move = normalized(aiInput.move);
    }

    float random01() {
        return std::uniform_real_distribution<float>(0.f, 1.f)(rng);
    }

    void showMessage(const std::string& text) {
        message = text;
        messageTimer = 1.2f;
    }

    void splash(sf::Vector2f pos, sf::Color color, int count) {
        for (int i = 0; i < count; ++i) {
            const float a = random01() * 6.28318f;
            const float s = 40.f + random01() * 210.f;
            particles.push_back({pos, {std::cos(a) * s, std::sin(a) * s}, color, 0.45f + random01() * 0.45f, 0.9f, 2.f + random01() * 5.f});
        }
    }

    void spawnAmbient(float dt) {
        static float acc = 0.f;
        acc += dt;
        if (acc > 0.08f) {
            acc = 0.f;
            particles.push_back({{random01() * WorldW, 80.f + random01() * 260.f}, {-15.f + random01() * 30.f, 25.f + random01() * 35.f}, {95, 150, 105}, 5.f, 5.f, 2.f});
        }
    }

    void updateParticles(float dt) {
        spawnAmbient(dt);
        for (auto& p : particles) {
            p.life -= dt;
            p.pos += p.vel * dt;
            p.vel *= 0.96f;
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return p.life <= 0.f; }), particles.end());
    }

    void render() {
        window.clear({22, 24, 24});
        drawBackground();
        if (screen == Screen::Battle || screen == Screen::Result) {
            drawFighter(ai);
            drawFighter(player);
            drawParticles();
            drawHud();
            if (screen == Screen::Result) {
                drawResult();
            }
        } else {
            drawParticles();
            drawMenu();
        }
        window.display();
    }

    void drawBackground() {
        sf::RectangleShape sky({WorldW, WorldH});
        sky.setFillColor({30, 36, 39});
        window.draw(sky);

        sf::CircleShape moon(58.f);
        moon.setPosition(940.f, 70.f);
        moon.setFillColor({218, 221, 200, 210});
        window.draw(moon);

        for (int i = 0; i < 22; ++i) {
            sf::RectangleShape bamboo({7.f, 460.f});
            bamboo.setOrigin(3.5f, 460.f);
            bamboo.setPosition(static_cast<float>(i) * 65.f + 10.f, 590.f);
            bamboo.setRotation((i % 5 - 2) * 2.5f);
            bamboo.setFillColor(i % 2 == 0 ? sf::Color{45, 82, 65} : sf::Color{38, 70, 58});
            window.draw(bamboo);
        }

        sf::RectangleShape floor({WorldW, 175.f});
        floor.setPosition(0.f, 545.f);
        floor.setFillColor({58, 63, 60});
        window.draw(floor);
        for (int i = 0; i < 13; ++i) {
            sf::RectangleShape line({WorldW, 1.f});
            line.setPosition(0.f, 560.f + static_cast<float>(i) * 12.f);
            line.setFillColor({83, 86, 80, 105});
            window.draw(line);
        }
    }

    void drawParticles() {
        for (const auto& p : particles) {
            const float ratio = clampf(p.life / p.maxLife, 0.f, 1.f);
            sf::CircleShape dot(p.radius * ratio);
            dot.setOrigin(p.radius * ratio, p.radius * ratio);
            dot.setPosition(p.pos);
            dot.setFillColor(withAlpha(p.color, static_cast<sf::Uint8>(190.f * ratio)));
            window.draw(dot);
        }
    }

    void drawFighter(const Fighter& f) {
        sf::Vector2f drawPos = {f.pos.x, f.pos.y - f.z};
        sf::CircleShape shadow(36.f);
        shadow.setOrigin(36.f, 12.f);
        shadow.setScale(1.3f, 0.34f);
        shadow.setPosition(f.pos.x, f.pos.y + 34.f);
        shadow.setFillColor({0, 0, 0, 95});
        window.draw(shadow);

        sf::RectangleShape body({50.f, 96.f});
        body.setOrigin(25.f, 82.f);
        body.setPosition(drawPos);
        body.setFillColor(f.action == Action::Hit ? sf::Color{245, 245, 245} : f.character.color);
        body.setOutlineThickness(3.f);
        body.setOutlineColor({18, 18, 18});
        window.draw(body);

        sf::CircleShape head(21.f);
        head.setOrigin(21.f, 21.f);
        head.setPosition(drawPos.x, drawPos.y - 94.f);
        head.setFillColor({232, 222, 205});
        head.setOutlineThickness(2.f);
        head.setOutlineColor({25, 25, 25});
        window.draw(head);

        sf::RectangleShape blade({72.f, 7.f});
        blade.setOrigin(8.f, 3.5f);
        blade.setPosition(drawPos.x + static_cast<float>(f.facing) * 24.f, drawPos.y - 54.f);
        blade.setScale(static_cast<float>(f.facing), 1.f);
        blade.setFillColor(f.weapon.id == "broadsword" ? sf::Color{185, 190, 185} : sf::Color{220, 225, 218});
        window.draw(blade);

        if (f.action == Action::Charging) {
            sf::CircleShape aura(48.f + f.chargeTimer * 18.f);
            aura.setOrigin(aura.getRadius(), aura.getRadius());
            aura.setPosition(drawPos.x, drawPos.y - 42.f);
            aura.setFillColor({0, 0, 0, 0});
            aura.setOutlineThickness(3.f);
            aura.setOutlineColor(withAlpha(f.character.color, 150));
            window.draw(aura);
        }
        if (f.action == Action::Parry) {
            sf::CircleShape ring(62.f);
            ring.setOrigin(62.f, 62.f);
            ring.setPosition(drawPos.x, drawPos.y - 42.f);
            ring.setFillColor({0, 0, 0, 0});
            ring.setOutlineThickness(5.f);
            ring.setOutlineColor({245, 245, 245, 190});
            window.draw(ring);
        }
    }

    void drawHud() {
        drawBars(player, 24.f, 22.f, false);
        drawBars(ai, WorldW - 324.f, 22.f, true);

        drawText(centerText(timeText(), 28), WorldW * 0.5f, 28.f, 28, {235, 230, 215});
        if (messageTimer > 0.f) {
            drawText(centerText(message, 46), WorldW * 0.5f, 145.f, 46, {245, 245, 235});
        }
    }

    std::string timeText() const {
        std::ostringstream oss;
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(std::ceil(std::max(0.f, matchTime)));
        return oss.str();
    }

    void drawBars(const Fighter& f, float x, float y, bool right) {
        const float w = 300.f;
        drawText(f.character.name + " / " + f.weapon.name, x, y, 18, {240, 236, 220});
        bar(x, y + 30.f, w, 16.f, f.hp / f.character.hp, {180, 45, 50});
        bar(x, y + 52.f, w, 10.f, f.energy / 100.f, {70, 145, 230});
        bar(x, y + 68.f, w, 10.f, f.stamina / 100.f, {220, 180, 70});
        if (right) {
            sf::RectangleShape frame({w, 78.f});
            frame.setPosition(x, y);
            frame.setFillColor({0, 0, 0, 0});
            frame.setOutlineThickness(1.f);
            frame.setOutlineColor({230, 230, 220, 80});
            window.draw(frame);
        }
    }

    void bar(float x, float y, float w, float h, float ratio, sf::Color color) {
        sf::RectangleShape bg({w, h});
        bg.setPosition(x, y);
        bg.setFillColor({18, 18, 18, 190});
        window.draw(bg);
        sf::RectangleShape fg({w * clampf(ratio, 0.f, 1.f), h});
        fg.setPosition(x, y);
        fg.setFillColor(color);
        window.draw(fg);
    }

    void drawMenu() {
        std::string title = "INK BLADE";
        std::vector<std::string> options;
        std::string subtitle;
        if (screen == Screen::MainMenu) {
            subtitle = "2D wuxia mind-game fighter";
            options = {"Start Duel", "Practice", "Exit"};
        } else if (screen == Screen::CharacterSelect) {
            subtitle = "Choose character";
            for (const auto& c : Characters) options.push_back(c.name + " - " + c.title);
        } else if (screen == Screen::WeaponSelect) {
            subtitle = "Choose weapon";
            for (const auto& w : Weapons) options.push_back(w.name + " - " + w.style);
        } else if (screen == Screen::DifficultySelect) {
            subtitle = "Choose AI difficulty";
            options = {"Easy", "Normal", "Hard"};
        }

        drawText(centerText(title, 58), WorldW * 0.5f, 92.f, 58, {242, 238, 220});
        drawText(centerText(subtitle, 22), WorldW * 0.5f, 166.f, 22, {190, 196, 185});
        for (std::size_t i = 0; i < options.size(); ++i) {
            const bool active = static_cast<int>(i) == menuIndex;
            sf::RectangleShape item({520.f, 48.f});
            item.setOrigin(260.f, 24.f);
            item.setPosition(WorldW * 0.5f, 260.f + static_cast<float>(i) * 70.f);
            item.setFillColor(active ? sf::Color{225, 222, 205, 220} : sf::Color{15, 15, 15, 120});
            item.setOutlineThickness(2.f);
            item.setOutlineColor(active ? sf::Color{120, 20, 20} : sf::Color{230, 230, 220, 80});
            window.draw(item);
            drawText(centerText(options[i], 24), WorldW * 0.5f, 245.f + static_cast<float>(i) * 70.f, 24, active ? sf::Color{25, 25, 25} : sf::Color{240, 235, 220});
        }

        if (screen == Screen::CharacterSelect) {
            drawText(Characters[menuIndex].passive, 410.f, 520.f, 18, {235, 230, 220});
            drawText(Characters[menuIndex].skill, 410.f, 550.f, 18, {235, 230, 220});
            drawText(Characters[menuIndex].ultimate, 410.f, 580.f, 18, {235, 230, 220});
        }
        if (screen == Screen::WeaponSelect) {
            drawText("Combo stages: " + std::to_string(Weapons[menuIndex].combo.size()), 470.f, 520.f, 18, {235, 230, 220});
            drawText("Parry window and counter differ per weapon", 470.f, 550.f, 18, {235, 230, 220});
        }
    }

    void drawResult() {
        sf::RectangleShape veil({WorldW, WorldH});
        veil.setFillColor({0, 0, 0, 165});
        window.draw(veil);
        drawText(centerText(showWin ? "VICTORY" : "DEFEAT", 64), WorldW * 0.5f, 170.f, 64, showWin ? sf::Color{235, 230, 205} : sf::Color{215, 80, 80});
        drawText(centerText("Enter: rematch    Esc: menu", 24), WorldW * 0.5f, 250.f, 24, {230, 226, 210});
        drawText("Damage: " + std::to_string(static_cast<int>(player.damageDone)), 500.f, 330.f, 22, {235, 230, 220});
        drawText("Parries: " + std::to_string(player.parries), 500.f, 365.f, 22, {235, 230, 220});
        drawText("Max combo: " + std::to_string(player.maxCombo), 500.f, 400.f, 22, {235, 230, 220});
        drawText("Charge hits: " + std::to_string(player.chargeHits), 500.f, 435.f, 22, {235, 230, 220});
        drawText("Dodges: " + std::to_string(player.dodges), 500.f, 470.f, 22, {235, 230, 220});
    }

    std::string centerText(const std::string& text, int px) const {
        const int spaces = std::max(0, static_cast<int>((WorldW * 0.5f - static_cast<float>(text.size() * px) * 0.25f) / 8.f));
        return std::string(static_cast<std::size_t>(spaces), ' ') + text;
    }

    void drawText(const std::string& text, float x, float y, unsigned size, sf::Color color) {
        static const sf::Font* font = nullptr;
        static sf::Font loaded;
        static bool attempted = false;
        if (!attempted) {
            attempted = true;
            const std::array<std::string, 4> paths = {
                "C:/Windows/Fonts/arial.ttf",
                "C:/Windows/Fonts/segoeui.ttf",
                "C:/Windows/Fonts/simhei.ttf",
                "C:/Windows/Fonts/msyh.ttc"
            };
            for (const auto& p : paths) {
                if (loaded.loadFromFile(p)) {
                    font = &loaded;
                    break;
                }
            }
        }
        if (!font) {
            return;
        }
        sf::Text t(text, *font, size);
        t.setPosition(x, y);
        t.setFillColor(color);
        window.draw(t);
    }
};

} // namespace

int main() {
    Game game;
    game.run();
    return 0;
}

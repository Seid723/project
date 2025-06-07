#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include "test.h"

class Emergency;

class Situation {
public:
    virtual void respond(Emergency &e) = 0;
    virtual std::string description() const = 0;
    virtual ~Situation() = default;
};

class Emergency {
    double health;
    double panic_level;
    double fire_damage;
    double flood_damage;
    double injury_level;
    std::vector<Situation *> response_plan;

public:
    Emergency(double _health, double _panic, double _fire, double _flood, double _injury, const std::vector<Situation *> &plan)
        : health(_health), panic_level(_panic), fire_damage(_fire), flood_damage(_flood), injury_level(_injury), response_plan(plan) {}

    double getHealth() const { return health; }
    void setHealth(double h) { health = h; }

    double getPanic() const { return panic_level; }
    void setPanic(double p) { panic_level = p; }

    double getFireDamage() const { return fire_damage; }
    void setFireDamage(double f) { fire_damage = f; }

    double getFloodDamage() const { return flood_damage; }
    void setFloodDamage(double f) { flood_damage = f; }

    double getInjuryLevel() const { return injury_level; }
    void setInjuryLevel(double i) { injury_level = i; }

    void activate() {
        for (auto *s : response_plan) {
            s->respond(*this);
        }
    }

    Emergency(const Emergency &) = delete;
    Emergency &operator=(const Emergency &) = delete;

    ~Emergency() {
        for (auto *s : response_plan)
            delete s;
    }
};

class Firefighters : public Situation {
    int units;
public:
    Firefighters(int u) : units(u) {}
    void respond(Emergency &e) override {
        double impact = std::min(units * 0.1, 1.0);
        e.setFireDamage(e.getFireDamage() * (1 - impact));
        e.setPanic(e.getPanic() * (1 - impact / 2));
    }
    std::string description() const override {
        return "Firefighters with " + std::to_string(units) + " units";
    }
};

class Medics : public Situation {
    int staff;
public:
    Medics(int s) : staff(s) {}
    void respond(Emergency &e) override {
        double heal = std::min(staff * 0.2, 1.0);
        e.setInjuryLevel(e.getInjuryLevel() * (1 - heal));
        e.setHealth(e.getHealth() + heal * 100);
    }
    std::string description() const override {
        return "Medics with " + std::to_string(staff) + " staff";
    }
};

class RescueTeam : public Situation {
    int boats;
public:
    RescueTeam(int b) : boats(b) {}
    void respond(Emergency &e) override {
        double effect = std::min(boats * 0.1, 1.0);
        e.setFloodDamage(e.getFloodDamage() * (1 - effect));
        e.setPanic(e.getPanic() * (1 - effect));
    }
    std::string description() const override {
        return "Rescue Team with " + std::to_string(boats) + " boats";
    }
};

class DelayedResponse : public Situation {
    Situation *inner;
    int delayRounds;
    int currentRound = 0;
public:
    DelayedResponse(Situation *s, int delay) : inner(s), delayRounds(delay) {}
    void respond(Emergency &e) override {
        if (++currentRound >= delayRounds) {
            inner->respond(e);
        }
    }
    std::string description() const override {
        return "Delayed: " + inner->description();
    }
    ~DelayedResponse() override {
        delete inner;
    }
};

TEST(EmergencyTest, Initialization) {
    std::vector<Situation *> plan = {
        new Firefighters(5),
        new Medics(3),
        new RescueTeam(2)
    };

    Emergency e(70, 40, 60, 50, 30, plan);

    ASSERT_EQ(e.getHealth(), 70);
    ASSERT_EQ(e.getPanic(), 40);
    ASSERT_EQ(e.getFireDamage(), 60);
    ASSERT_EQ(e.getFloodDamage(), 50);
    ASSERT_EQ(e.getInjuryLevel(), 30);

    return true;
}

TEST(EmergencyTest, ResponseEffect) {
    std::vector<Situation *> plan = {
        new Firefighters(5),
        new Medics(3),
        new RescueTeam(2)
    };

    Emergency e(70, 40, 60, 50, 30, plan);
    e.activate();

    ASSERT_TRUE(e.getFireDamage() < 60);
    ASSERT_TRUE(e.getFloodDamage() < 50);
    ASSERT_TRUE(e.getPanic() < 40);
    ASSERT_TRUE(e.getHealth() > 70);
    ASSERT_TRUE(e.getInjuryLevel() < 30);

    return true;
}

TEST(EmergencyTest, DelayedResponse) {
    std::vector<Situation *> plan = {
        new DelayedResponse(new Firefighters(5), 2)
    };

    Emergency e(70, 40, 60, 50, 30, plan);
    e.activate();
    ASSERT_EQ(e.getFireDamage(), 60);
    e.activate();
    ASSERT_TRUE(e.getFireDamage() < 60);

    return true;
}

TEST(EmergencyTest, ZeroUnitsResponse) {
    std::vector<Situation *> plan = {
        new Firefighters(0),
        new Medics(0),
        new RescueTeam(0)
    };

    Emergency e(70, 40, 60, 50, 30, plan);
    e.activate();

    ASSERT_EQ(e.getFireDamage(), 60);
    ASSERT_EQ(e.getFloodDamage(), 50);
    ASSERT_EQ(e.getPanic(), 40);
    ASSERT_EQ(e.getHealth(), 70);
    ASSERT_EQ(e.getInjuryLevel(), 30);

    return true;
}

TEST(EmergencyTest, MaxEffectCap) {
    std::vector<Situation *> plan = {
        new Firefighters(20),
        new Medics(10),
        new RescueTeam(20)
    };

    Emergency e(70, 40, 60, 50, 30, plan);
    e.activate();

    ASSERT_TRUE(e.getFireDamage() <= 0);
    ASSERT_TRUE(e.getFloodDamage() <= 0);
    ASSERT_TRUE(e.getPanic() < 40);
    ASSERT_TRUE(e.getHealth() > 70);
    ASSERT_TRUE(e.getInjuryLevel() < 30);

    return true;
}

int main() {
    RUN_TEST(EmergencyTest, Initialization);
    RUN_TEST(EmergencyTest, ResponseEffect);
    RUN_TEST(EmergencyTest, DelayedResponse);
    RUN_TEST(EmergencyTest, ZeroUnitsResponse);
    RUN_TEST(EmergencyTest, MaxEffectCap);
    return 0;
}

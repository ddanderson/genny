#include "test.h"

#include <gennylib/Orchestrator.hpp>
#include <iostream>

using namespace genny;

namespace {

std::thread start(Orchestrator& o, int phase) {
    return std::thread{[&o, phase]() {
        o.awaitPhaseStart();
        REQUIRE(o.currentPhaseNumber() == phase);
    }};
}

std::thread end(Orchestrator& o, int phase) {
    return std::thread{[&o, phase]() {
        REQUIRE(o.currentPhaseNumber() == phase);
        o.awaitPhaseEnd();
    }};
}

}  // namespace

TEST_CASE("Orchestrator") {
    auto o = Orchestrator{};
    ActorVector vec;
    vec.push_back({});
    vec.push_back({});
    o.setActors(vec);

    REQUIRE(o.currentPhaseNumber() == 0);
    REQUIRE(o.morePhases());

    auto t1 = start(o, 0);
    auto t2 = start(o, 0);
    t1.join();
    t2.join();

    REQUIRE(o.currentPhaseNumber() == 0);
    REQUIRE(o.morePhases());

    auto t3 = end(o, 0);
    auto t4 = end(o, 0);
    t3.join();
    t4.join();

    REQUIRE(o.currentPhaseNumber() == 1);
    REQUIRE(o.morePhases());

    // now all wait for phase 1

    auto t5 = start(o, 1);
    auto t6 = start(o, 1);
    t5.join();
    t6.join();

    REQUIRE(o.currentPhaseNumber() == 1);
    REQUIRE(o.morePhases());

    auto t7 = end(o, 1);
    auto t8 = end(o, 1);
    t7.join();
    t8.join();

    REQUIRE(o.currentPhaseNumber() == 2);
    REQUIRE(!o.morePhases());
}

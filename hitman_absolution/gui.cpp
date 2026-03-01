#include "gui.hpp"

#include "../hitman_common/gui.hpp"

const std::vector<std::string> map_names = {
    // A Personal Contract
    "#1 Garden",
    "#2 Greenhouse",
    "#3 Cliffside",
    "#4 Mansion Ground Floor",
    "#5 Mansion 2nd Floor",
    // The King of Chinatown
    "#6 The King of Chinatown",
    // Terminus
    "#7 Terminus Hotel",
    "#8 Upper Floors",
    // Run for Your Life
    "#9 Burning Hotel",
    "#10 The Libary",
    "#11 Pigeon Coop",
    "#12 Rooftops",
    "#13 Shangri-La",
    "#14 Train Station",
    // Hunter and Hunted
    "#15 Countryard",
    "#16 The Vixen Club",
    "#17 Dressing Rooms",
    "#18 Derelict Building",
    "#19 Convenience Store",
    "#20 Loading Area",
    "#21 Chinese New Year",
    // Rosewood
    "#22 Victoria's Ward",
    "#23 Orphanage Halls",
    "#24 Central Heating",
    // Welcome To Hope
    "#25 Great Balls Of Fire",
    // Birdie's Gift
    "#26 Gun Shop",
    // Shaving Lenny
    "#27 Streets Of Hope",
    "#28 Barbershop",
    "#29 The Desert",
    // Dexter Industries
    "#30 Dead End",
    "#31 Old Mill",
    "#32 Descent",
    "#33 Factory Compound",
    // Death Factory
    "#34 Test Facility",
    "#35 Decontamination",
    "#36 R&D",
    // Fight Night
    "#37 Patriots Hangar",
    "#38 Arena",
    // Attack of the Saints
    "#39 Parking",
    "#40 Reception",
    "#41 Cornfield",
    // Skurky's Law
    "#42 Courthouse",
    "#43 Holding Cells",
    "#44 Prison",
    // Operation Sledgehammer
    "#45 County Jail",
    "#46 Outgunned",
    "#47 Burn",
    "#48 Hope Fair",
    "#49 Church",
    // Tailor Shop
    "#50 Tailor Shop",
    // Blackwater Park
    "#51 Blackwater Park",
    "#52 The Penthouse",
    "#53 Blackwater Roof",
    // Absolution
    "#54 Cementary Entrance",
    "#55 Burnwood Family Tomb",
    "#56 Crematorium",
};

void hitman_absolution::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        "Hitman: Absolution",
        stats.difficulty == 0   ? "Easy"
        : stats.difficulty == 1 ? "Normal"
        : stats.difficulty == 2 ? "Hard"
        : stats.difficulty == 3 ? "Expert"
                                : "Purist",
        map_names,
        stats.rating.value != "Unrated" ? 
        std::vector<hitman_common::TableRow>{
            {"Innocents Killed", stats.innocents_killed},
            {"Enemies Killed", stats.enemies_killed},
            {"Spotted", stats.alerts},
            {"Evidence Left", stats.on_camera},
            {"Objectives Left", stats.objectives_left},
            {"Pacifications", stats.pacifications},
            {"Bodies Hidden", stats.bodies_hidden},
            {"Headshots", stats.headshots},
            {"Silent Kills", stats.silent_kills},
            {"Signature Kills", stats.signature_kills},
            {"Silent Assassin Bonus", stats.sa_bonus},
        } : std::vector<hitman_common::TableRow>{}
    );
}

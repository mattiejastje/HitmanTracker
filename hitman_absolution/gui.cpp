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
    "#9 Room 899",
    // Run for Your Life
    "#10 Burning Hotel",
    "#11 The Libary",
    "#12 Pigeon Coop",
    "#13 Rooftops",
    "#14 Shangri-La",
    "#15 Train Station",
    // Hunter and Hunted
    "#16 Countryard",
    "#17 The Vixen Club",
    "#18 Dressing Rooms",
    "#19 Derelict Building",
    "#20 Convenience Store",
    "#21 Loading Area",
    "#22 Chinese New Year",
    // Rosewood
    "#23 Victoria's Ward",
    "#24 Orphanage Halls",
    "#25 Central Heating",
    // Welcome To Hope
    "#26 Great Balls Of Fire",
    // Birdie's Gift
    "#27 Gun Shop",
    // Shaving Lenny
    "#28 Streets Of Hope",
    "#29 Barbershop",
    "#30 The Desert",
    // Dexter Industries
    "#31 Dead End",
    "#32 Old Mill",
    "#33 Descent",
    "#34 Factory Compound",
    // Death Factory
    "#35 Test Facility",
    "#36 Decontamination",
    "#37 R&D",
    // Fight Night
    "#38 Patriots Hangar",
    "#39 Arena",
    // Attack of the Saints
    "#40 Parking",
    "#41 Reception",
    "#42 Cornfield",
    // Skurky's Law
    "#43 Courthouse",
    "#44 Holding Cells",
    "#45 Prison",
    // Operation Sledgehammer
    "#46 County Jail",
    "#47 Outgunned",
    "#48 Burn",
    "#49 Hope Fair",
    "#50 Outskirts",
    "#51 Church",
    // Tailor Shop
    "#52 Tailor Shop",
    // Blackwater Park
    "#53 Blackwater Park",
    "#54 The Penthouse",
    "#55 Blackwater Roof",
    // Absolution
    "#56 Cementary Entrance",
    "#57 Burnwood Family Tomb",
    "#58 Crematorium",
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

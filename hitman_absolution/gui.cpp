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
    "#12 Shangri-La",
    "#13 Train Station",
    // Hunter and Hunted
    "#14 Countryard",
    "#15 The Vixen Club",
    "#16 Dressing Rooms",
    "#17 Derelict Building",
    "#18 Convenience Store",
    "#19 Loading Area",
    "#20 Chinese New Year",
    // Rosewood
    "#21 Victoria's Ward",
    "#22 Orphanage Halls",
    "#23 Central Heating",
    // Welcome To Hope
    "#24 Great Balls Of Fire",
    // Birdie's Gift
    "#25 GunShop",
    // Shaving Lenny
    "#26 Streets Of Hope",
    "#27 Barbershop",
    "#28 The Desert",
    // Dexter Industries
    "#29 Dead End",
    "#30 Old Mill",
    "#31 Descent",
    "#32 Factory Compound",
    // Death Factory
    "#33 Test Facility",
    "#34 Decontamination",
    "#35 R&D",
    // Fight Night
    "#36 Patriots Hangar",
    "#37 Arena",
    // Attack of the Saints
    "#38 Parking",
    "#39 Reception",
    "#40 Cornfield",
    // Skurky's Law
    "#41 Courthouse",
    "#42 Holding Cells",
    "#43 Prison",
    // Operation Sledgehammer
    "#44 County Jail",
    "#45 Outgunned",
    "#46 Burn",
    "#47 Hope Fair",
    "#48 Church",
    // Tailor Shop
    "#49 Tailor Shop",
    // Blackwater Park
    "#50 Blackwater Park",
    "#51 The Penthouse",
    "#52 Blackwater Roof",
    // Absolution
    "#53 Cementary Entrance",
    "#54 Burnwood Family Tomb",
    "#55 Crematorium",
};

void hitman_absolution::gui(
    const settings::Gui& settings, const Fonts& fonts, const Stats& stats
) {
    hitman_common::gui(
        settings,
        fonts,
        stats,
        "Hitman: Absolution",
        "",
        map_names,
        {
            {"Spotted", stats.spotted},
        }
    );
}

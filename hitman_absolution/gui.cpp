#include "gui.hpp"

#include <imgui.h>

#include <format>

#include "../hitman_common/gui.hpp"
#include "../imgui_utils.hpp"

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
    "#16 Courtyard",
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

static settings::HMA::RatingMode get_rating_mode(
    const settings::HMA& hma, CheckpointType checkpoint_type
) {
    switch (checkpoint_type) {
        case CheckpointType::NO_TARGETS:
            return hma.rating_mode_no_targets;
        case CheckpointType::TARGETS:
            return hma.rating_mode_targets;
        default:
            return hma.rating_mode_unrated;
    };
}

GameGui hitman_absolution::gui(
    const settings::Gui& settings,
    const settings::HMA& hma,
    const std::string& version
) {
    return [&settings, &hma, version](const Fonts& fonts, const Stats& stats) {
        auto game_name = settings.show_game_version
                             ? std::format("{} [{}]", GAME_NAME, version)
                             : GAME_NAME;
        hitman_common::gui_header(
            settings,
            fonts,
            game_name,
            stats.difficulty == 0   ? "Easy"
            : stats.difficulty == 1 ? "Normal"
            : stats.difficulty == 2 ? "Hard"
            : stats.difficulty == 3 ? "Expert"
                                    : "Purist",
            map_names[stats.map],
            stats.map,
            stats.map_stage,
            stats.time
        );
        auto rating_mode = get_rating_mode(hma, stats.checkpoint_type);
        auto is_silent_assassin = stats.rating.status == Status::GREEN;
        auto is_sa_shown
            = rating_mode == settings::HMA::RatingMode::SA
              || rating_mode == settings::HMA::RatingMode::SA_PLUS_SC
              || (rating_mode == settings::HMA::RatingMode::SA_FALLBACK_SC
                  && is_silent_assassin);
        auto is_sc_shown
            = rating_mode == settings::HMA::RatingMode::SC
              || rating_mode == settings::HMA::RatingMode::SA_PLUS_SC
              || (rating_mode == settings::HMA::RatingMode::SA_FALLBACK_SC
                  && !is_silent_assassin);
        if (is_sa_shown) {
            hitman_common::gui_table(
                settings,
                fonts,
                stats.rating,
                stats.map,
                stats.map_stage,
                {
                    {"Non-Target Casualty", stats.score_non_target_casualty},
                    {"Spotted", stats.score_spotted},
                }
            );
        }
        if (is_sc_shown) {
            auto is_max = stats.score_total >= stats.score_for_max_rating;
            auto rating_status = is_max ? Status::GREEN : Status::YELLOW;
            auto rating_font = is_max ? fonts.rating_good : fonts.rating_maybe;
            auto rating_color = is_max ? settings.rating_good.color
                                       : settings.rating_maybe.color;
            std::string rating_text;
            rating_text = std::format(
                "{} [{}/{}]",
                stats.score_rating,
                stats.score_total,
                stats.score_for_max_rating
            );
            std::vector<hitman_common::TableRow> table_rows{};
            if (!is_sa_shown) {
                table_rows.emplace_back(
                    "Non-Target Casualty", stats.score_non_target_casualty
                );
                table_rows.emplace_back("Spotted", stats.score_spotted);
            }
            table_rows.emplace_back(
                "Civilian Casualty", stats.score_civilian_casualty
            );
            table_rows.emplace_back("Pacification", stats.score_pacification);
            table_rows.emplace_back("Body Hidden", stats.score_body_hidden);
            table_rows.emplace_back("Headshot", stats.score_headshot);
            table_rows.emplace_back("Silent Kill", stats.score_silent_kill);
            table_rows.emplace_back(
                "Objective Complete", stats.score_objective_complete
            );
            // status on evidence removed not set on maps with no evidence
            if (stats.score_evidence_removed.status) {
                table_rows.emplace_back(
                    "Evidence Removed", stats.score_evidence_removed
                );
            }
            // status on silent asssassin bonus is not set on maps with no
            // targets
            if (stats.score_silent_assassin_bonus.status) {
                table_rows.emplace_back(
                    "Signature Kill", stats.score_signature_kill
                );
                table_rows.emplace_back("Target Kill", stats.score_target_kill);
                table_rows.emplace_back(
                    "Silent Assassin Bonus", stats.score_silent_assassin_bonus
                );
            }
            hitman_common::gui_table(
                settings,
                fonts,
                StatsValue{rating_text, rating_status},
                stats.map,
                stats.map_stage,
                table_rows
            );
        }
    };
}
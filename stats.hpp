#pragma once

#include <cinttypes>

enum SilentAssassin { YES, MAYBE, NO };

struct Stats {
    float time;
    int map;
    SilentAssassin silent_assassin;
    int32_t difficulty;
    int32_t shots_fired;
    int32_t headshots;
    int32_t enemies_killed;
    int32_t enemies_wounded;
    int32_t innocents_killed;
    int32_t innocents_wounded;
    int32_t close_encounters;   // h2sa
    int32_t alerts;             // h2sa
    int32_t police_killed;      // bm
    int32_t police_wounded;     // bm
    int32_t frisk_failed;       // bm
    int32_t cover_blown;        // bm
    int32_t bodies_fnd;         // bm
    int32_t target_bodies_fnd;  // bm
    int32_t uncon_bodies_fnd;   // bm
    int32_t witnesses;          // bm
    int32_t on_camera;          // bm
    int32_t cust_weapons_left;  // bm
    int32_t suit_left;          // bm
};

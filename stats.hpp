#pragma once

#include <cinttypes>

enum SilentAssassin { YES, MAYBE, NO };

struct Stats {
    float time;
    int map;
    SilentAssassin silent_assassin;
    uint32_t difficulty;
    uint32_t shots_fired;
    uint32_t headshots;
    uint32_t enemies_killed;
    uint32_t enemies_wounded;
    uint32_t innocents_killed;
    uint32_t innocents_wounded;
    uint32_t close_encounters;   // h2sa
    uint32_t alerts;             // h2sa
    uint32_t police_killed;      // bm
    uint32_t police_wounded;     // bm
    uint32_t frisk_failed;       // bm
    uint32_t cover_blown;        // bm
    uint32_t bodies_fnd;         // bm
    uint32_t target_bodies_fnd;  // bm
    uint32_t uncon_bodies_fnd;   // bm
    uint32_t witnesses;          // bm
    uint32_t on_camera;          // bm
    uint32_t cust_weapons_left;  // bm
    uint32_t suit_left;          // bm
};

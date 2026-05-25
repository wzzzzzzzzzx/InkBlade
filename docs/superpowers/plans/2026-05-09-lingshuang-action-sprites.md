# Ling Shuang Action Sprites Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Move Ling Shuang's Win32 rendering toward a reusable action-sprite table and add PNG sprites for normal attack, charging, charge release, skill, and ultimate.

**Architecture:** Keep the current Win32/GDI renderer and WIC PNG loader, but replace the growing set of one-off Ling Shuang bitmap fields with an indexed action sprite table. The first implementation remains scoped to Ling Shuang so the same pattern can later be reused for Mo Hen and Su Xin.

**Tech Stack:** C++17, Win32 API, WIC PNG loading, GDI AlphaBlend, PowerShell validation and visual smoke scripts.

---

### Task 1: Add Failing Integration Check

**Files:**
- Create: `scripts/check_lingshuang_action_sprite_table.ps1`

- [ ] Write a PowerShell check that requires `normal_01.png`, `charging_01.png`, `charge_release_01.png`, `skill_01.png`, and `ultimate_01.png` under `assets/art/characters/lingshuang/sprites/body`.
- [ ] Require `LingShuangSpriteSlot`, `lingshuangSprites`, `loadLingShuangActionSprites`, and action mappings for `Action::Normal`, `Action::Charging`, `Action::ChargeRelease`, `Action::Skill`, and `Action::Ultimate`.
- [ ] Run the check and confirm it fails on missing assets before implementation.

### Task 2: Extract Runtime PNGs

**Files:**
- Create: `scripts/extract_lingshuang_action_sprites.ps1`
- Create outputs in: `assets/art/characters/lingshuang/sprites/body`

- [ ] Crop five clear poses from `assets/art/characters/lingshuang/concept/lingshuang_action_pose_sheet.png`.
- [ ] Export each sprite to a 768x768 transparent PNG using the same edge-connected background removal as existing extraction scripts.
- [ ] Inspect generated sprites and tighten crop rectangles if adjacent pose fragments remain.

### Task 3: Replace One-Off Ling Shuang Bitmap Fields

**Files:**
- Modify: `src/main_win32.cpp`

- [ ] Add a scoped enum `LingShuangSpriteSlot` for idle, run1, run2, jump, dodge, parry, hit, normal, charging, charge release, skill, and ultimate.
- [ ] Replace individual `lingshuangIdle`, `lingshuangRun1`, etc. fields with `std::array<BitmapImage, ...> lingshuangSprites`.
- [ ] Add `loadLingShuangActionSprites()` to load all Ling Shuang body PNGs into the table.
- [ ] Update the destructor to destroy every bitmap in the table.

### Task 4: Select New Action Sprites

**Files:**
- Modify: `src/main_win32.cpp`

- [ ] Update `drawLingShuangSprite()` to choose new sprites for normal, charging, charge release, skill, and ultimate states.
- [ ] Preserve existing run, jump, dodge, parry, hit, and idle behavior.
- [ ] Keep fallback behavior to idle if a PNG is missing.

### Task 5: Upgrade Visual Smoke

**Files:**
- Modify: `scripts/smoke_interaction.ps1`

- [ ] Add screenshots for Ling Shuang normal attack, charging/charge release, skill, and ultimate where input can be triggered deterministically.
- [ ] Keep the 9 character/weapon combination smoke coverage.
- [ ] Build, package, run checks, run GUI smoke, and inspect the most important screenshots.

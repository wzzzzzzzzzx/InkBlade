# Weapon Battle Readability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the three weapon assets visibly distinct in battle and align their visual placement with combat readability checks.

**Architecture:** Keep the current single-file Win32 renderer and existing asset folders. Add small weapon render layout data, use existing `model_01.png` files, and extend self-tests/scripts so weapon sprites, debug hitboxes, and weapon-specific visual transforms remain covered.

**Tech Stack:** C++17 Win32/GDI+, PowerShell regression scripts, existing PNG art assets.

---

### Task 1: Weapon Render Layout

**Files:**
- Modify: `src/main_win32.cpp`
- Test: `scripts/check_weapon_sprite_integration.ps1`

- [ ] **Step 1: Write/extend the regression check**

Add source markers to `scripts/check_weapon_sprite_integration.ps1` for `WeaponRenderLayout`, `weaponRenderLayout`, and `drawWeaponSprite`. Expected: the script fails before the code contains those markers.

- [ ] **Step 2: Implement render layout data**

Add a compact `WeaponRenderLayout` struct near `WeaponActionTuning`, with per-weapon width and per-action offsets for idle, run, normal, charging, charge release, parry, dodge, skill, ultimate, and hit/down fallback.

- [ ] **Step 3: Use the layout in `drawWeaponSprite`**

Replace the current hard-coded weapon sprite widths/positions with `weaponRenderLayout(f, weapon)`, preserving the existing procedural fallback when the PNG is missing.

- [ ] **Step 4: Verify**

Run:
`powershell -ExecutionPolicy Bypass -File .\scripts\check_weapon_sprite_integration.ps1`

Expected:
`OK weapon sprite integration checks passed.`

### Task 2: Combat Debug Readability

**Files:**
- Modify: `src/main_win32.cpp`
- Test: `scripts/check_weapon_tuning_debug.ps1`

- [ ] **Step 1: Extend debug drawing**

In `drawFighterDebug`, keep the body rectangle and attack rectangle, and add a center line from fighter position toward attack range. Use distinct colors for player and enemy.

- [ ] **Step 2: Extend self-test coverage**

Update `runWeaponTuningSelfTest()` to check that debug hitboxes are hidden by default and visible when `practiceDebugHitboxes` is true, and that weapon render layouts have distinct widths.

- [ ] **Step 3: Verify**

Run:
`powershell -ExecutionPolicy Bypass -File .\scripts\check_weapon_tuning_debug.ps1`

Expected:
`OK weapon tuning/debug checks passed.`

### Task 3: Build, Package, Smoke Test

**Files:**
- Modify only files touched by Tasks 1-2.

- [ ] **Step 1: Build**

Run:
`powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1`

Expected: build exits 0 and produces `build\InkBlade.exe`.

- [ ] **Step 2: Run focused regressions**

Run:
`powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\check_combat_balance_regression.ps1`
`powershell -ExecutionPolicy Bypass -File .\scripts\check_weapon_sprite_integration.ps1`
`powershell -ExecutionPolicy Bypass -File .\scripts\check_weapon_tuning_debug.ps1`
`powershell -ExecutionPolicy Bypass -File .\scripts\check_visual_lab_mode.ps1`

Expected: all scripts exit 0.

- [ ] **Step 3: Package and smoke**

Run:
`Stop-Process -Name InkBlade -Force -ErrorAction SilentlyContinue; powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1`
`powershell -ExecutionPolicy Bypass -File .\scripts\smoke_interaction.ps1`

Expected: package succeeds and smoke reaches battle for all 9 character/weapon combinations.

- [ ] **Step 4: Commit and push**

Run:
`git add src/main_win32.cpp scripts/check_weapon_sprite_integration.ps1 scripts/check_weapon_tuning_debug.ps1 docs/superpowers/plans/2026-06-05-weapon-battle-readability.md`
`git commit -m "Improve weapon battle readability"`
`git push origin main`

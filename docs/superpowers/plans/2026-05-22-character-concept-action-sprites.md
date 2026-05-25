# Character Concept Action Sprites Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace MoHen and SuXin temporary action sprites with cleaner sprites extracted from their original concept action sheets, and add real run frames.

**Architecture:** Add a focused asset extraction script that crops known cells from each character action sheet, removes the light sheet background, trims transparent padding, and writes runtime sprite filenames. Keep runtime rendering code mostly unchanged, except undoing the temporary MoHen weapon-overlay skip when concept sprites are weaponless.

**Tech Stack:** C++17 Win32 runtime, PowerShell verification scripts, Python/Pillow for one-off PNG extraction.

---

### Task 1: Extract Concept-Based Body Sprites

**Files:**
- Create: `scripts/extract_concept_action_sprites.py`
- Modify assets under `assets/art/characters/mohen/sprites/body/`
- Modify assets under `assets/art/characters/suxin/sprites/body/`

- [ ] Create a script that crops the 4x2 action pose sheets.
- [ ] Map sheet cells to runtime slots:
  - MoHen: idle cell 0, run_01 cell 1, run_02 cell 3, jump cell 2, normal cell 4, hit cell 5, skill cell 6, ultimate cell 7.
  - SuXin: idle cell 0, run_01 cell 1, run_02 cell 3, jump cell 2, normal cell 4, hit cell 5, skill cell 6, ultimate cell 7.
- [ ] Remove near-white/light gray background and crop transparent margins.
- [ ] Save refined versioned files and runtime files.

### Task 2: Reduce Body-Embedded Effects

**Files:**
- Modify generated sprite outputs from Task 1.

- [ ] Keep only character body, clothing, hair, ribbons, and very small contact marks.
- [ ] Avoid importing large swirls, lotus rings, or ink circles into body sprite files.
- [ ] Leave heavy VFX in independent VFX layers.

### Task 3: Undo Temporary MoHen Weapon Workaround

**Files:**
- Modify: `src/main_win32.cpp`

- [ ] Remove the temporary `spriteCarriesWeapon` condition for MoHen normal, charging, and charge release.
- [ ] Let weapons render consistently over all MoHen and SuXin body sprites again.

### Task 4: Verify

**Files:**
- Use existing scripts.

- [ ] Run static checks:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\check_visual_lab_mode.ps1`
  - `powershell -ExecutionPolicy Bypass -File .\scripts\check_character_visual_profiles.ps1`
  - `powershell -ExecutionPolicy Bypass -File .\scripts\check_character_battle_art_spec.ps1`
- [ ] Run build and package:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1`
  - `powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1`
- [ ] Run visual and interaction checks:
  - `powershell -ExecutionPolicy Bypass -File .\scripts\visual_combat_test.ps1`
  - `powershell -ExecutionPolicy Bypass -File .\scripts\smoke_interaction.ps1`

# InkBlade Character Battle Art Spec v1

This is the machine-checkable companion spec for the Chinese art document `角色战斗素材规格.md`.

## Baseline

- Ling Shuang is the current quality baseline.
- Formal character body sprites must be single-character transparent PNGs.
- Do not use cropped pose-sheet panels, UI frames, white backgrounds, corner marks, borders, or dirty matte edges as final battle assets.
- Character feet must remain visible because the feet define the ground anchor.
- The character silhouette must stay readable at combat scale.

## Runtime Body Sprite Paths

```text
assets/art/characters/lingshuang/sprites/body/
assets/art/characters/mohen/sprites/body/
assets/art/characters/suxin/sprites/body/
```

Runtime filenames:

```text
idle_01.png
run_01.png
run_02.png
jump_01.png
dodge_01.png
parry_01.png
normal_01.png
charging_01.png
charge_release_01.png
hit_01.png
skill_01.png
ultimate_01.png
```

Runtime filenames are the only body sprites loaded by the game. Refined or chroma-key staging files are temporary and should not be treated as required runtime assets. After a staging asset passes screenshot review, copy it to the runtime filename and delete the disposable staging file unless it documents an approved art decision.

Concept-sheet extraction filenames:

```text
idle_01_concept_01.png
run_01_concept_01.png
run_02_concept_01.png
jump_01_concept_01.png
dodge_01_concept_01.png
normal_01_concept_01.png
charging_01_concept_01.png
charge_release_01_concept_01.png
hit_01_concept_01.png
skill_01_concept_01.png
ultimate_01_concept_01.png
```

MoHen and SuXin baseline movement and combat-body sprites should be extracted from their original `concept/*_action_pose_sheet.png` files before generating new poses. Keep body sprites mostly character-only; large ink, lotus, wind, or aura shapes should stay in independent VFX layers unless they are tiny contact marks.

## Size Hygiene

- Keep source concept sheets, select portraits, final runtime PNGs, concept-extracted backups, scripts, docs, and independent VFX layers.
- Delete disposable chroma-key sources, failed refined variants, temporary trim outputs, and visual-lab screenshots once newer validation images exist.
- Do not store multiple large failed variants in `assets/`; keep only the current accepted runtime body sprite and its concept-extracted backup.

## VFX Split Target

Ultimate art should eventually be split into body and VFX:

```text
assets/art/characters/<character>/sprites/body/ultimate_01.png
assets/art/characters/<character>/sprites/vfx/ultimate_vfx_01.png
```

Body art holds the character pose. VFX art holds ice, ink, lotus, petals, wind rings, and other large effects.
When a character has `ultimate_vfx_01.png`, the runtime draws the body pose and VFX as separate layers.

## Chroma Key Generation

Use `#00ff00` or `#ff00ff` only as removable generation backgrounds. The character itself must not contain the chosen key color.

Prompt marker:

```text
original 2D side-view wuxia fighting game character sprite, flat #00ff00 or #ff00ff chroma-key background, full body, feet visible, no frame, no text, no watermark
```

## Visual Validation

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check_suxin_death_resolution.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check_character_visual_profiles.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check_character_battle_art_spec.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\check_visual_lab_mode.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\package.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\visual_combat_test.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\smoke_interaction.ps1
```

Review:

```text
build/visual-lab/visual_lab_char0_idle.png
build/visual-lab/visual_lab_char1_idle.png
build/visual-lab/visual_lab_char2_idle.png
```

Before accepting a replacement body sprite, inspect the matching visual-lab screenshot at combat scale:

- Feet, hat, hair, sleeves, ribbons, and weapon silhouettes must not be cropped.
- Idle feet must have a small transparent bottom margin so the character does not look cut off on the ground.
- Character scale must stay consistent across idle, run, dodge, parry, hit, skill, and ultimate.
- Skill props such as SuXin's lotus must not have hollow alpha holes unless they are intentional VFX shapes.
- Large aura, ink, ice, lotus, or wind effects should be split into `sprites/vfx/` instead of baked into every body sprite.

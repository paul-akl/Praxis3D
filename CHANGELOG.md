# Changelog

All notable changes to Praxis3D are documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
Version numbers match `PRAXIS3D_VERSION_*` in [`Version.hpp`](Praxis3D/Source/Common/Include/Version.hpp).

## Template

Copy this block when a version is released and fill in only the headings that apply.

```markdown
## [X.Y.Z] - YYYY-MM-DD

### Added
- New feature, one short line. (#123)

### Changed
- Existing behavior that was modified.

### Fixed
- Bug that was resolved.

### Removed
- Anything deleted.

### Deprecated
- Anything scheduled for removal.
```

Guidelines:

- One line per entry, written in the past tense, no trailing period.
- Keep entries user-facing; skip internal refactors unless they change behavior.
- Drop any heading that has no entries.
- Use the release date from `Version.hpp`.

---

## [0.2.7] - 2026-11-26

### Added
- Mouse object picking in the scene editor.
- CMake presets for Debug and Release builds.

### Changed
- Reorganized the source file structure.
- Enabled the Ninja generator for CMake builds.

### Fixed
- Missing `PickingRenderPass` in the default scene.
- Assets occasionally not finishing loading.
- Runtime errors from unpaired ImGui calls and screen-space triangle VAO unloading.
- Debug and Release builds.
- Clangd warning.

### Removed
- Unused files and the `/permissive` compiler flag.

---

## [0.2.6] - 2024-04-06

### Added
- Cylinder physics body.

### Fixed
- Assorted bugs.

---

## [0.2.5] - 2024-02-26

### Added
- FXAA screen-space anti-aliasing.
- Texture compression with downsampling, tint color and 2D scaling.
- Stochastic texture sampling.
- Cascaded shadow mapping.
- Ambient occlusion (SSAO and HBAO).
- Text editor and texture inspector.
- "About" window with font loading and sound event playback.
- Per-mesh face-cull settings.
- Default models, textures and sounds.

### Changed
- Rendering optimizations.

### Fixed
- Bloom ordering.

---

[Unreleased]: https://github.com/paul-akl/Praxis3D/compare/v0.2.7...HEAD
[0.2.7]: https://github.com/paul-akl/Praxis3D
[0.2.6]: https://github.com/paul-akl/Praxis3D/releases/tag/v0.2.6
[0.2.5]: https://github.com/paul-akl/Praxis3D/releases/tag/v0.2.5

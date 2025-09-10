## Purpose

Short, practical guidance for AI coding agents working on this ESP32 + LVGL project (EEZ Studio UI). Use this file to get productive quickly and avoid changing generated artifacts.

## Big picture (what the project is)

- It's an ESP-IDF project (target: esp32s3). LVGL-based UI created with EEZ Studio. Main entry: `main/main.c`.
- Architectural pattern: simple MVC with a Bridge layer that connects EEZ-generated view code to app logic.
  - View (generated): `main/view/src_ui/` (contains `screens.c` and other EEZ outputs). DO NOT EDIT these files manually.
  - Controller / Bridge: `main/controller/` (files named `ui_*_bridge.c`, e.g. `ui_weather_bridge.c`, `ui_wifi_bridge.c`). Bridges implement EEZ actions and call into model code.
  - Model: `main/model/` (app logic, e.g. `app_weather.c`, `app_wifi.c`, `settings.c`).

Data flow summary: `screens.c` (EEZ) -> EEZ action handlers -> `ui_*_bridge.c` -> `app_*.c` (model) -> update UI via bridge/ui_tick.

Key files to inspect:
- `main/main.c` — app lifecycle, LVGL init, `ui_init()`, `ui_tick()`, and use of `bsp_display_lock/unlock` around UI ops.
- `main/CMakeLists.txt` — how sources are discovered: view sources are globbed from `view/src_ui`, controllers from `controller`, models from `model`.
- `sdkconfig` — project configuration for ESP-IDF (many defaults already set).
- `managed_components/` — contains third-party drivers and LVGL; avoid editing these directly.

Conventions and patterns (concrete)
- EEZ Studio generated files are authoritative for UI layout and action names. If a UI control/event is missing, modify in EEZ Studio and regenerate rather than editing `view/src_ui`.
- Bridge files follow `ui_<feature>_bridge.c` and expose init/process functions: `ui_bridge_init()`, `ui_bridge_process_*_updates()`.
- LVGL concurrency: UI access must be wrapped by `bsp_display_lock(0)` / `bsp_display_unlock()` as shown in `main.c`.
- Language: UI strings and many comments are Spanish in this repo; preserve locale and style when adding UI text.
- Adding source files: place new controllers in `main/controller/` or models in `main/model/`. They are picked up by `main/CMakeLists.txt` (GLOB_RECURSE for `view/src_ui` and GLOB for `controller`/`model`).

Build / test / debug workflows (Windows PowerShell examples)
- Typical build (requires ESP-IDF environment already set up):
  - idf.py build
  - idf.py -p <PORT> flash monitor
  - The build artifacts live under `build/` (e.g. `build/lvgl_demos.bin`, `bootloader/bootloader.bin`).
- SDK config: `sdkconfig` is checked in; to change settings run `idf.py menuconfig` then rebuild.
- If you need to inspect logs on-device use `idf.py -p <PORT> monitor` (or your serial monitor). Flashing and monitor are managed via `idf.py` (ESP-IDF standard).

Integration points / external dependencies
- ESP-IDF (project uses CMake-based ESP-IDF project model via `$IDF_PATH`). See top-level `CMakeLists.txt` and `main/CMakeLists.txt`.
- LVGL and related drivers live in `managed_components/` (e.g. `lvgl__lvgl`, `espressif__esp_lcd_touch`, `espressif__esp_lcd_panel_io_additions`). Don't modify upstream managed components in-place; prefer updating via the component manager or PRs to the source.
- Board support & BSP helpers: `bsp/` APIs are used (e.g. `bsp_display_start()`, `bsp_display_lock()`), so changes to BSP affect UI timing and locking.

Quick examples (copyable patterns found in repo)
- Respect LVGL lock:

  bsp_display_lock(0);
  ui_tick();
  ui_bridge_process_wifi_updates();
  bsp_display_unlock();

- Add a new controller file: drop `my_feature.c` into `main/controller/` and implement an init function; it will be compiled automatically by `main/CMakeLists.txt`.

What agents must NOT do
- Do not edit files under `main/view/src_ui/` — they are EEZ Studio generated.
- Do not modify checked-in managed components without explicit instruction from the repo owners.

Where to look first when debugging a UI issue
- `main/main.c` (startup and main loop)
- `main/view/src_ui/screens.c` (generated handlers wiring to actions)
- `main/controller/ui_*_bridge.c` (bridge logic)
- `main/model/app_*.c` (network, weather, settings behavior)

If something is missing or unclear
- Ask the human: should the UI be changed in EEZ Studio or should the bridge adapt? The repo prefers changing the UI in EEZ Studio.

Please review these notes and tell me which sections you want expanded or any missing local conventions to include.

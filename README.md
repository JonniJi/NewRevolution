# NewRevolution

A rotation-driven LED ring for an **Adafruit Feather ESP32-S2**. A hall sensor sees a magnet go past on a spinning wheel, and each pass moves the pattern one step, so the light follows the motion. A potentiometer picks the animation.

> **Status:** imported unchanged from `Revolution_ESP32_S2` in [ride4sun/ArduidoProjects](https://github.com/ride4sun/ArduidoProjects) (files dated 2022-08-22). It hasn't been compiled or run since the import. It has known bugs, listed in [Known issues](#known-issues) below. This is the base for future work.

---

## How it works

1. **Wheel turns → pattern steps.** The hall sensor triggers an interrupt (on every change, but only every second edge counts, which gives one event per magnet). Each event moves the ring's position by one LED and redraws the active animation.
2. **Parked → fake rotation.** If the sensor sees nothing for 5 seconds, the code fakes an event every 100 ms (`SIMULATED_INTERRUPT_TIME`), so the ring keeps moving while the vehicle is still. One lap of 150 LEDs then takes about 15 s.
3. **Knob → animation.** Once a second the potentiometer is read and mapped to one of the animation slots.
4. **Startup:** pixels 0–3 light red, orange, yellow, green, 2.5 s apart (about 10 s total). Then the pattern starts.

Each animation is one file in `src/` and implements the `IAnimation` interface from `src/defines.h`:

| Method | When it runs |
|---|---|
| `OnSetup()` | once, at boot |
| `OnHallEvent(ledData)` | on every wheel step (real or faked) and draws the frame |
| `OnFastLoop()` | every loop (unused by all animations so far) |
| `Name()` | the name printed to Serial |

## Hardware

All of these are set in [`src/defines.h`](src/defines.h).

| Setting | Value | Notes |
|---|---|---|
| Board | Adafruit Feather ESP32-S2 | `platformio.ini`: `board = featheresp32-s2` (chip ESP32-S2FN4R2) |
| LED type | WS2812B, colour order GRB | APA102 option commented out |
| Ring one | 150 LEDs on **pin 2** | test ring; `306` for the "big Ring on Golfcart" is commented out |
| Ring two | 52 LEDs on **pin 4** | off; turn on with `#define LED_STRING_TWO_PRESENT` |
| Hall sensor | **pin 3**, input with pull-up | |
| Potentiometer | **A0** | |
| Brightness | 100 of 255 | **no current limit set**, see issue 4 |
| Serial | 115200 baud | |

### Options in `defines.h`

| Define | Effect |
|---|---|
| `LED_STRING_TWO_PRESENT` | Drive the second (inner) ring |
| `AUTO_SELECT_ANIMATION` | Cycle animations every 30 s instead of using the knob (broken, see issue 9) |
| `SHOW_POTENTIOMETER_INFO` | Print knob readings (on) |
| `SHOW_POSITION_PRINT_INFO` | Print ring position on every step |
| `SHOW_LOCK_AND_QUEUE_INFO` | Print interrupt queue info (prints from inside the interrupt, debug only) |

## Animations (ring one)

The knob ranges were written for a 0–1023 reading. The ESP32-S2 reads 0–8191, so as the code stands, only the bottom eighth of the knob does anything (issue 2).

| Slot | Knob reading | Animation | Looks like (intended) |
|---|---|---|---|
| 0 | 0–49 | Rotate, two-colour fade | 16 spots, alternating blue/red, with trails |
| 1 | 51–148 | Rotate, one-colour fade | 16 blue spots with trails |
| 2 | 150–199 | Rotate, two-colour fade | as 0, shorter trails |
| 3 | 201–299 | Rotate, one-colour fade | as 1, shorter trails |
| 4 | 301–399 | Rotate, two-colour | 8 blue/red spots, no trails |
| 5 | 401–499 | Rotate, two-colour | identical to 4 |
| 6 | 501–599 | Rotate, rainbow | 8 rainbow spots |
| 7 | 601–699 | Juggle | 8 coloured dots weaving |
| 8 | 701–799 | CandyCane | blue and black stripes moving with the wheel |
| 9 | 801–899 | BlueAndWhite | alternating pixels (broken, issue 6) |
| 10 | (none) | Sinelon | a dot with a fading trail (no knob range points here) |
| 11 | 901–1199 | **empty** | **crashes**, issue 2 |

In stock but not in the list: HeartBeat, BPM, Rainbow, Rainbow2, WhiteDotRunning, Snake.

## Building

The project uses **[PlatformIO](https://platformio.org/)** (VS Code extension). Libraries come from `platformio.ini`:

- `fastled/FastLED@^3.5.0` (the `^` means "newest 3.x", so today's build gets a much newer FastLED than the 2022 one)
- `slocomptech/QList@^0.6.7`

PlatformIO isn't installed on the development machine yet. Fix issue 5 before the first build.

---

## Known issues

From a code read on 2026-10-03, without compiling or running anything. Line numbers refer to the imported version. Fix the **critical** ones before putting this on a real ring.

### Critical: can crash the board or overload the power

- [ ] **1. Animation setup may never run.** `src/main.cpp:131` and `:138`: `for (int i; …)` never sets `i` to 0. If setup is skipped, Rotate's spot spacing (`gap`) is never calculated and its spots land in random places. *Fix:* `for (int i = 0; …)`.
- [ ] **2. Knob ranges are wrong for the ESP32-S2 and one crashes.** `src/main.cpp:184–194`, `:206–216`. The ranges assume a 0–1023 reading, but the S2 reads 0–8191. Slot `[10]` is never filled in, and slot `[11]` (901–1199) points to an animation that doesn't exist, so turning the knob there **crashes**. Readings of exactly 50, 149, 200 and so on fall between ranges. *Fix:* work out the slot from the reading (e.g. `map()` over the real count of animations) instead of a hand-written table.
- [ ] **3. The interrupt isn't safe.** `src/main.cpp:78–106`. `interruptHall()` calls `QList::push_back`, which reserves memory, and that isn't safe inside an interrupt on an ESP32. The main loop pops from the same list with no protection (the `lock` flag doesn't block interrupts). Shared flags aren't `volatile`, and the routine isn't `IRAM_ATTR`. This can cause random resets once a real magnet is passing. *Fix:* in the interrupt, only add 1 to a `volatile` counter. The loop reads and clears it with interrupts paused.
- [ ] **4. No current limit.** At brightness 100, full white is about 3.5 A for 150 LEDs and about 7 A for the 306-LED ring. *Fix:* add `FastLED.setMaxPowerInVoltsAndMilliamps(5, …)` sized to the real supply.
- [ ] **5. `platformio.ini` probably won't load.** Line 1 starts with a stray `device;`. *Fix:* delete the word.

### Animations not doing what they're meant to

- [ ] **6. BlueAndWhite shows almost nothing.** `src/blueAndWhite.hpp:12–13` stores the colours as `uint8_t`, which loses the colour. It's also called with `HUE_RED`, which is a hue number, not a colour. It only draws every 3rd pixel and never clears the rest. *Fix:* store `CRGB`, pass real colours.
- [ ] **7. Rotate spots have gaps.** `src/rotate.hpp:64`: `currentPos = (currentPos + i)` adds up as it goes, so a 3-pixel spot lights 0, 1 and 3. *Fix:* `(start + i) % n`.
- [ ] **8. Rotate wipes the other ring.** `src/rotate.hpp:52`: `FastLED.clear()` blanks every strip. This matters once ring two is on. *Fix:* `fill_solid(data.leds, data.noOfLeds, CRGB::Black)`.
- [ ] **9. Auto-select doesn't switch animations.** `src/main.cpp:288–293` changes `potPosition` but never updates `activeAnimationOne`.
- [ ] **10. The "when stationary" set is unused.** `src/main.cpp:30–31`, `:217`, `:371–381`. It's filled in but never drawn. This was probably meant to play while the wheel is still.
- [ ] **11. BPM never pulses.** `src/bpm.hpp:12` works out the beat once at startup. Each step lights one pixel and nothing fades, so the ring just fills up.
- [ ] **12. Leftover variables.** `src/heartBeat.hpp:21` (a beat at 0 BPM, then `gHue` is overwritten), `src/rainbow2.hpp:17` (`rainbowHue` is never used, so the rainbow stands still), `src/sinelon.hpp:18` (`pos` is never used). The animations don't match their names.
- [ ] **13. Snake loops forever.** `src/snake.hpp:49`: `for (uint16_t i = …; i >= 0; i--)` never ends (an unsigned number is never below 0) and writes past the end of the LED array. It's not used now. Fix it before turning it on.

### Small things

- [ ] Messages printed before `Serial.begin()` are lost (`src/main.cpp:424` runs before `:434`).
- [ ] `uint16_t potVal = analogRead(A0);` (`src/main.cpp:74`) reads the pin before the board has started up.
- [ ] `numberOfAnimations = 10` (`src/main.cpp:420`) but 11 are defined. Count them from the list instead.
- [ ] `ESP32_S2FN4R2.code-workspace` points to a folder on another computer (`C:\Users\ride4\…`).
- [ ] `rotate.hpp:41` sets `toggleColor` to itself (does nothing). `LED_BUILTIN` is set up but never used.
- [ ] The startup animation blocks for about 10 s with `delay()`.
- [ ] `HeartBeat` and `Sinelon` pass `noOfLeds - 1` into an 8-bit value, which overflows on the 306-LED ring.

## Ideas for later

- Measure wheel speed (time between hall events) and use it to change tempo or colour, not only position.
- Finish the stationary set: one look while moving, another while parked.
- Bring it onto the same toolchain as [led-piece](https://github.com/JonniJi/led-piece) (arduino-cli, pinned versions) if the two projects merge.

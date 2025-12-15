# 🎓 Electronic Cheat Sheet (ESP32-C6-LCD-1.47)

A simple electronic cheat sheet with a graphical interface powered by the Waveshare ESP32-C6 microcontroller and LVGL library.

## 🛠 Hardware

*   **MCU:** [Waveshare ESP32-C6-LCD-1.47](https://www.waveshare.com/esp32-c6-lcd-1.47.htm) (RISC-V, 1.47" TFT 172x320).
*   **Controls:** 2 tactile buttons.
*   **Storage:** MicroSD Card (formatted FAT32).
*   **Pinout:**
    *   **Button 1 (Prev/Select):** `GPIO 0` ↔ `GND`
    *   **Button 2 (Next/Back):** `GPIO 1` ↔ `GND`
    *   *Note:* The code uses `INPUT_PULLUP`, so external resistors are not required.

## 💻 Software Stack

*   **IDE:** Arduino IDE (v2.x+).
*   **UI Framework:** LVGL (Light and Versatile Graphics Library).
*   **UI Builder:** SquareLine Studio.
*   **Libraries:**
    *   `lvgl`
    *   `EncButton` (for button handling)
    *   `SD` (Standard Arduino SD library)
    *   Driver libraries (usually included in the project folder like `Display_ST7789.h`).

## 🚀 Installation & Setup

1.  **Arduino IDE Setup:**
    *   Install ESP32 Board support (version 3.0.0+ is required for C6).
    *   Select Board: `ESP32C6 Dev Module`.
    *   **Important:** Set `USB CDC On Boot` to `"Enabled"` to use the Serial Monitor.

2.  **Dependencies:**
    Install `EncButton` and `lvgl` via the Arduino Library Manager. Ensure local project files (`Display_ST7789.h`, `SD_Card.h`, `ui.h`, etc.) are in the sketch folder.

3.  **SD Card:**
    *   Format a MicroSD card to FAT32.
    *   Create `.txt` files in the root directory.
    *   Insert the card into the slot.

4.  **Flashing:**
    *   Connect the device via USB.
    *   Hold the `BOOT` button (GPIO 0) while pressing `RESET` to enter Download Mode if automatic upload fails.
    *   Upload the sketch.

## 🎮 Controls

| Button | Action | Function in Menu | Function in Reading Mode |
| :--- | :--- | :--- | :--- |
| **GPIO 0 (Left)** | **Click** | Scroll Up / Previous File | Previous Page/Fragment |
| **GPIO 0 (Left)** | **Hold** | **Return to Menu** | **Return to Menu** |
| **GPIO 1 (Right)** | **Click** | Scroll Down / Next File | Next Page/Fragment |
| **GPIO 1 (Right)** | **Hold** | **Open Selected File** | (No action / Reload) |

## ⚠️ Notes

1.  **Text Format:** The reader expects standard `.txt` files.
2.  **Pagination:** The reader splits long lines into 400-character chunks to fit the small screen.
3.  **GPIO 0 Conflict:** Since GPIO 0 is also the Boot strap pin, ensure the button is **not pressed** when powering on or resetting the device, otherwise, it will enter bootloader mode instead of running your code.

---
**Author:** Pundemia
**Date:** 2025

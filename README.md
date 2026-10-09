# ESP32 BLE Advertising & Modern OS Security Analysis

A technical case study and Proof-of-Concept (PoC) evaluating Bluetooth Low Energy (BLE) stack stability, controller-level dynamic MAC address spoofing on ESP32, and security mitigations implemented in modern operating systems (iOS, Android, Windows).

---

## [!] Project Overview

The primary objective of this research was to analyze how modern mobile and desktop operating systems react to unauthenticated BLE proximity advertising packets (such as Apple AirDrop/AirPods pop-ups, Google Fast Pair, and Microsoft Swift Pair).

Rather than serving as a functional "spam tool", this project documents low-level firmware engineering challenges (specifically RAM/Heap management within the ESP-IDF framework) and demonstrates why modern OS-level security features successfully suppress unauthenticated pop-up triggers.

---

## [*] Technical Implementation & Architecture

### 1. Stack Stability & Memory Management
- **Problem:** The standard ESP32 `Bluedroid` library continuously allocates heap memory during rapid advertising payload updates, eventually resulting in `CORRUPT HEAP` crashes and kernel panics. This issue is particularly critical on resource-constrained architectures like the ESP32.
- **Solution:** Replaced `Bluedroid` with **`NimBLE-Arduino`**, significantly reducing memory footprint and eliminating heap corruption during continuous broadcast loops.

### 2. MAC Address Filtering & Rate Limiting (OS Thresholds)
- **Problem:** Modern operating systems (iOS/Android) enforce rate-limiting mechanisms against repetitive BLE proximity packets originating from a static MAC address. Sequential packets from the same MAC address are categorized under a single session and silently dropped by the OS.
- **Solution Intent:** To ensure each broadcast cycle is recognized as originating from a new physical source, updating the Media Access Control (MAC) address prior to every payload transmission became mandatory.

### 3. Controller-Level Dynamic MAC Spoofing
- Re-initializing the Bluetooth stack to cycle the MAC address introduces substantial latency and CPU overhead.
- **Implementation:** The firmware configures the address type as `BLE_ADDR_RANDOM` (`Random Static Address`) and updates controller-level random bytes using `esp_fill_random()` before every payload iteration, achieving seamless MAC rotation without stack restarts.

---

## Comparative Analysis & Related Work

To understand the boundaries of BLE security research, this project evaluates two distinct paradigms in the open-source community:

1. **Software-Defined Advertising Floods (e.g., [tobozo/ESP32-BLEBeaconSpam](https://github.com/tobozo/ESP32-BLEBeaconSpam)):**
   - **Approach:** Relies purely on brute-force advertising packet generation using standard ESP32 BLE stacks.
   - **Limitation:** Fails against modern OS mitigations (iOS rate-limiting, Android ECC verification) as unauthenticated packets are dropped at the driver/protocol level.

2. **Hybrid Hardware RF Generators (e.g., [CiferTech nRFBox](https://www.hackster.io/CiferTech/esp32-powered-tool-to-scan-jam-spoof-ble-wi-fi-nrfbox-96b516)):**
   - **Approach:** Expands ESP32 capability using external 2.4GHz transceivers (nRF24L01) to perform physical layer (PHY) RF noise generation and multi-protocol spoofing.
   - **Limitation:** While effective at disrupting physical radio signal quality (PHY noise), external transceivers still cannot bypass cryptographic authentication or OS-level pop-up suppression logic once packets reach the target device's Bluetooth stack.

---

## [?] OS-Level Security Mitigations & Findings

RF spectrum analysis conducted via **nRF Connect for Mobile** verified that the ESP32 successfully broadcasts valid raw RF packets and rotates MAC addresses in real-time (~300 ms interval, -57 dBm RSSI). However, target operating systems deliberately ignore these packets due to modern security protocols:

- **iOS (Apple):** iOS 17.2+ strictly rate-limits proximity pop-ups and enforces cryptographic verification on manufacturer data payloads.
- **Android (Google Fast Pair):** Fast Pair triggers require a valid `Model ID` registered via Google Nearby Console and elliptic-curve cryptography (ECC) signature verification. Unsigned packets are silently discarded at the OS level.
- **Windows (Swift Pair):** Swift Pair notifications are suppressed in the background unless the Bluetooth settings interface is actively opened by the user.

---

## [X] Limitations & Future Work

The key protocol barriers identified during this study that could not be bypassed due to hardware/software constraints include:

* **Google Fast Pair (Android Authenticated Signal Request):**
  * **Unresolved Barrier:** Even if a valid `Model ID` is included in the payload, Google's ecosystem demands an Elliptic Curve Cryptography (ECC) signature generated via a Private Key. Devices unverified by Google Nearby Console cannot trigger background pop-ups.
  * **Future Work:** Investigating authorized device handshake packets at the RF level using sniffing / MITM techniques.

* **Apple Proximity Anti-Spam (iOS Rate-Limiting):**
  * **Unresolved Barrier:** Apple applies dynamic rate-limiting within its background BLE scanning service. Even when rotating MAC addresses, if the Manufacturer ID (`0x004C`) remains static during rapid bursts, iOS flags the behavior as an anomaly/spam and suppresses the notification layer.
  * **Future Work:** Reverse engineering transient state flags and encrypted payload structures within Apple AirDrop/AirPods protocol frames.

* **Windows Swift Pair Passive Scanning Limits:**
  * **Unresolved Barrier:** Windows suppresses UI-level notification pop-ups during passive background scanning. Packets are processed into UI elements only when the system is explicitly placed in active pairing mode (Discoverability Window).

---

## [+] Verification & Test Setup

- **Hardware:** ESP32 Development Board
- **Development Environment:** PlatformIO / C++
- **BLE Library:** NimBLE-Arduino
- **Packet Sniffer / RF Verification:** nRF Connect for Mobile (Android/iOS)
- **Measured Signal Strength (RSSI):** ~ -57 dBm to -60 dBm

---

## [>] Build & Flash Instructions

1. Open the project directory in **VS Code** with the **PlatformIO** extension installed.
2. Connect your ESP32 board via USB.
3. Build and upload the project using PlatformIO.
4. Open the Serial Monitor (`115200` baud rate) to observe real-time payload logs.
5. Use **nRF Connect** on a nearby smartphone to observe live RF broadcasts and dynamic MAC address rotations.

---

## [i] License

This project is licensed under the MIT License - see the `LICENSE` file for details.

# 🛸 ESP-Drone - DIY Mini Drone with ESP Microcontrollers

[![License: Apache License 2.0](https://img.shields.io/github/license/saltstack/salt)](LICENSE)
[![ESP32](https://img.shields.io/badge/Microcontroller-ESP32-green.svg)](https://github.com/espressif/esp-idf)
[![Project Stage: Experimental](https://img.shields.io/badge/stage-experimental-orange.svg)](https://github.com/bhanujgunas/ESP-drone)

> A lightweight, low-cost, and highly educational DIY mini drone project built using ESP microcontrollers, tailored for embedded systems and RC enthusiasts.

---

## 📦 Base Components

### 🔧 Drone Side
- 🧱 Mini drone frame  
- ⚙️ 8520 Coreless Brushed Motors (x4)  
- 🌀 55mm / 65mm propellers  
- 🔋 1S LiPo Battery (3.7V, 1000mAh, 30C)  
- 🛠️ Motor driver (MX1508 initially, then custom AO3400-based)  
- 🧠 ESP32 Development Board (Receiver & Flight Controller)  
- 🧭 MPU6050 (Gyroscope & Accelerometer)

### 🎮 Transmitter Side
- 🎮 Dual analog joystick modules (x2)  
- 🔳 Breadboard (for prototyping)  
- 📡 ESP8266 (Transmitter)  
- 🖥️ 1.3" OLED Display  
- 📈 ADS1115 (16-bit ADC for joysticks)

### 🧰 Other Essential Components
- 🔌 LiPo charger  
- 🪛 Wires and jumpers  
- 🧠 Sufficient knowledge of microcontrollers & embedded systems  
- 🔧 Soldering tools & experience  
- 🔁 CP2102 / FTDI USB-to-Serial Converter  
- 🔩 Resistors (4.7kΩ for gate pull-downs)  
- 🧲 Capacitors  
- 📏 Optional: Propeller guards for testing safety

---

## 🔄 Development History

### ⚙️ Iteration One: MX1508 and Early Issues

Initially, the motors were fitted into the frame, but the motor wires were slightly damaged. I fixed this by sleeving all four motor wires for better durability and insulation.

I used the **MX1508 motor driver** to interface the motors with the ESP32. The ESP32 was intended as the receiver and controller, but I faced issues with libraries like `AsyncTCP`. For initial testing, I switched to an **ESP8266** for simplicity.

I powered the drone using a **1S 1000mAh 30C LiPo battery**, but it turned out to be **too heavy**, which affected flight performance.

<div align="center">
  <img src="<<img>>" alt="Iteration One Setup" height="300px"/>
</div>

---

### 🛠️ Iteration Two: Custom MOSFET Driver

After watching several community project videos, I decided to **build a custom lightweight motor driver** using:

- **AO3400 N-Channel MOSFETs**  
- **4.7kΩ resistors** (to pull the gate low)  
- **SS14 Schottky diodes** (for flyback protection)

This setup ensures the MOSFET turns **on only when the gate is pulled high**, and **reverse flyback currents are safely diverted**, protecting the ESP32 from potential damage.

I reused the same heavy battery (despite knowing better) since I was still in the testing phase.

I also mounted the **MPU6050**, though I didn’t integrate it into the motor control logic yet. All modules, including the ESP32 (with header pins), were **zip-tied** onto the frame. The header pins interfered with propellers, so adjustments were made to avoid contact.

#### 🧪 Issue: Limited Motor Speed
The motors did **not spin at full speed**, even at max throttle. Supplying power directly from the battery made them spin correctly, so I suspected either **ESP8266 or the code**.

I rewrote the code for **ESP32**, reassembled the drone, and zip-tied everything for further tests.

#### ⚠️ Dangerous Moment
One test went wrong — I uploaded a **faulty code** that drove all motors at full throttle. It was around **6 AM**, and the drone was **hanging on a USB cable**, spinning violently. I panicked, dropped it to the floor, and **damaged my fingers** with the propellers. Two props were damaged; one broke entirely.

Then I made a **fatal wiring mistake**: I connected the **charged 1S battery (4.1V)** to `VIN` while the **5V USB was also connected**. The battery slowly charged up to **4.35V**, and without a load to drain it, the **LiPo started puffing**. It continued puffing for days and had to be safely disposed of.

---

### 🔥 Iteration Three: Transmitter Build & Battery Disposal

While preparing a **custom transmitter**, I also decided to **safely dispose** of the puffed battery.

To conduct this safely, I moved to my **rooftop** and tried to pierce the battery to neutralize it.

<div align="center">
  <video src="<<video>>" controls height="300px"></video>
</div>

Meanwhile, I built the **new transmitter setup** using:
- ESP8266
- Dual joysticks + ADS1115 for analog reading
- OLED display for feedback

Testing continues, and I hope this **iteration three** results in a stable and flying prototype. Stay tuned.

---

## 📚 Lessons Learned

- Avoid using **overweight batteries** in mini drone builds.
- **Custom drivers** can greatly reduce weight but must be safely designed with **flyback protection**.
- Do **not power VIN and USB simultaneously** while a LiPo is connected — this causes overcharging.
- **Puffed LiPo batteries are dangerous** and should be disposed of following e-waste or controlled experiments (do not throw in regular bins).
- **Secure and organize all modules** properly to prevent propeller interference.

---

## 🚀 What's Next

- Integrate **PID stabilization** using MPU6050  
- Improve **ESP-NOW communication** between ESP8266 and ESP32  
- Add **telemetry feedback** and **fail-safe mechanisms**  
- Design a custom **PCB** for reduced weight and compactness  
- Test flight modes: **Manual**, **Stabilize**, and **Altitude Hold**

---

## 🛠️ Future Ideas

- Use **brushless motors (with ESCs)** for power efficiency  
- Add **camera + FPV transmission**  
- Integrate **altitude sensor** like BMP180  
- Support **GPS-based autonomous modes**

---

## 💡 Contributing

Pull requests are welcome! If you have suggestions for improvements, please open an issue or submit a PR.

---

## 📝 License

This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for more info.

---

> _"Experiment. Fail. Learn. Repeat."_  
> – The Drone Maker’s Journey 🚁

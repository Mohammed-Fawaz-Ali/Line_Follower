# Wiring — Detailed Notes

## ⚠️ About the L293D

> **The L293D is not the recommended choice for driving DC motors in robotics projects.**
>
> | Spec | L293D | L298N | TB6612FNG |
> |---|---|---|---|
> | Max current per channel | 600 mA | 2 A | 1.2 A |
> | Internal voltage drop | ~2.8 V total | ~3 V total | ~0.5 V |
> | Heat dissipation | Poor (no heatsink tab) | Good (module heatsink) | Excellent |
> | Package | DIP-16 (breadboard friendly) | Module | Module |
> | Verdict | ⚠️ Avoid for motors | ✅ Acceptable | ✅ Recommended |
>
> **Why it works here anyway:** N20 motors draw only ~100–250 mA under load, well within the 600 mA limit. The voltage drop hurts torque slightly but doesn't cause failure. The L293D was used here because it was available — for any future or upgraded build, switch to TB6612FNG.

---

## L293D Pinout (DIP-16)

```
          ┌────────────────┐
  EN1,2 ──┤ 1           16 ├── VCC1 (logic 5V)
  IN1   ──┤ 2           15 ├── IN4
  OUT1  ──┤ 3           14 ├── OUT4
  GND   ──┤ 4           13 ├── GND
  GND   ──┤ 5           12 ├── GND
  OUT2  ──┤ 6           11 ├── OUT3
  IN2   ──┤ 7           10 ├── IN3
  VCC2  ──┤ 8            9 ├── EN3,4
          └────────────────┘
   (motor supply)    (enable right)
```

---

## Full Connection Table

| L293D Pin | Connect To | Notes |
|---|---|---|
| Pin 1  — EN1,2 | 5V (Nano) | Keep HIGH to enable left H-bridge |
| Pin 2  — IN1 | D8 (Nano) | Left motor direction A |
| Pin 3  — OUT1 | Left Motor + | |
| Pin 4,5 — GND | Common GND | Both ground pins — connect both |
| Pin 6  — OUT2 | Left Motor − | |
| Pin 7  — IN2 | D9 (Nano, PWM) | Left motor direction B / speed |
| Pin 8  — VCC2 | Battery + | Motor supply: **5V–6V max** |
| Pin 9  — EN3,4 | 5V (Nano) | Keep HIGH to enable right H-bridge |
| Pin 10 — IN3 | D10 (Nano, PWM) | Right motor direction A / speed |
| Pin 11 — OUT3 | Right Motor + | |
| Pin 12,13 — GND | Common GND | Both ground pins — connect both |
| Pin 14 — OUT4 | Right Motor − | |
| Pin 15 — IN4 | D11 (Nano) | Right motor direction B |
| Pin 16 — VCC1 | 5V (Nano) | Logic supply — separate from VCC2 |

> ⚠️ **Connect all 4 GND pins** (4, 5, 12, 13). The L293D uses the GND pins as its heatsink path — missing even one increases heat and can cause thermal shutdown.

---

## Motor Direction Logic

For **LEFT MOTOR** (IN1=D8, IN2=D9, EN1,2=5V):

| IN1 (D8) | IN2 (D9) | Motor |
|---|---|---|
| HIGH | LOW | Forward (full speed) |
| HIGH | PWM | Forward (variable speed) |
| LOW | HIGH | Reverse |
| LOW | LOW | Coast |

For **RIGHT MOTOR** (IN3=D10, IN4=D11, EN3,4=5V):

| IN3 (D10) | IN4 (D11) | Motor |
|---|---|---|
| PWM | LOW | Forward (variable speed) |
| HIGH | LOW | Forward (full speed) |
| LOW | HIGH | Reverse |
| LOW | LOW | Coast |

### If a motor runs backwards
Swap the two wires going into that motor's OUT pins on the L293D (e.g. swap OUT1↔OUT2). Do **not** swap the IN pin wires.

---

## Power Architecture

```
Battery (4×AA = 6V, or 1S LiPo = 3.7V)
     └──► L293D Pin 8 (VCC2) ──► Motor supply
                                  (~2.8V drop → motors see ~3.2V from 6V)

Arduino Nano 5V pin ──► L293D Pin 16 (VCC1) ──► Logic supply
                    └──► L293D Pin 1 & 9    ──► Enable pins
                    └──► IR sensor VCC × 2

All GND → tied together (battery −, Nano GND, L293D pins 4,5,12,13)
```

> ⚠️ Do **not** power the Nano from a 9V PP3 battery — it cannot supply enough current for both motors and logic. Use 4×AA or a small Li-ion pack.

---

## Common Ground Rule

All GND connections must be tied together:
- Battery negative
- Arduino Nano GND
- L293D pins 4, 5, 12, 13
- Both IR sensor GND pins

Missing a common GND is the #1 cause of erratic motor behaviour.

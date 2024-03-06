# PI Terminal for Flipper Zero 
[Flipper Zero](https://flipperzero.one/) app to control various devices via UART interface.

## Capabilities
- Reading from UART in text or hex mode
- Sending commands
- Sending fast commands
- Sending binary packets (in hex)
- Baudrate selection
- UART pins selection (2 options)

## Connecting
| Device UART interface  | Flipper Zero pin (default) | Flipper Zero pin (option) |
| :----------------: | :---------------: | :---------------: |
| RX | 13 TX | 15 TX |
| TX | 14 RX | 16 TX |
| GND  | 8, 11, 18 GND | 8, 11, 18 GND |

Info: If possible, do not power your devices from 3V3 (pin 9) Flipper Zero. It does not support hot plugging.

## Keyboard
Pi_Terminal uses its own special keyboard for work, which has all the symbols necessary for working in the console.

To accommodate more characters on a small display, some characters are called up by holding.

![kbf](https://user-images.githubusercontent.com/122148894/212286637-7063f1ee-c6ff-46b9-8dc5-79a5f367fab1.png)

## Hex mode
Hexadecimal mode is useful for working with simple binary protocols.
You can see the bytes being received in hexadecimal in the terminal window.
You can also send binary packets to UART. A simplified keyboard is used to enter packet bytes.

![004](https://raw.githubusercontent.com/InfoSecREDD/Flip-pi/main/img/4.png)

## How to install
[Momemntum Firmware](https://github.com/Next-Flip/Momentum-Firmware)

or

[Unleashed Firmware](https://github.com/DarkFlippers/unleashed-firmware)

Copy the contents of the repository to the applications_user/Pi_Terminal folder Flipper Zero firmware and build app with the command ./fbt fap_Pi_Terminal.

Or use the tool [uFBT](https://github.com/flipperdevices/flipperzero-ufbt) for building applications for Flipper Zero.

## Raspberry Pi Zero UART

![Rpi_Zero_GPIO](https://user-images.githubusercontent.com/22322762/183289015-45329e9c-83e0-4483-96a0-1b68552bfa3f.png)

## Raspberry Pi Zero : Prerequisite

[Kali Linux](https://www.kali.org/get-kali/#kali-platforms) is preferred OS.

Mount your SD Card to your computer and add the following line at the end of `/boot/config.txt` file.

`enable_uart=1`

Additionally you can look at the baudrate in the `/boot/cmdline.txt` file, which should be `115200` by default.

## Raspberry Pi Zero : Pinout to Flipper Zero

```
PiZero: 4 (5V) -> FZ: 1 (5V)
PiZero: 6 (GND) -> FZ: 8, 11 or 18 (GND)
PiZero: 8 (UART0_TXD) -> FZ: 14 or 16 (RX)
PiZero: 10 (UART0_RXD) -> FZ: 13 or 15 (TX)
```

## How it works


![1f](https://raw.githubusercontent.com/InfoSecREDD/Flip-pi/main/img/1.png)


![2f](https://raw.githubusercontent.com/InfoSecREDD/Flip-pi/main/img/3.png)


![4f](https://raw.githubusercontent.com/InfoSecREDD/Flip-pi/main/img/2.png)

## INFO:
This project has been built off these two projects. Thank you for all your hard work! 
[Wifi Marauder](https://github.com/0xchocolate/flipperzero-firmware-with-wifi-marauder-companion),
[UART_Terminal](https://github.com/cool4uma/UART_Terminal)

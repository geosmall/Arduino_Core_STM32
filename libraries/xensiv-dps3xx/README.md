# XENSIV™ Digital Pressure Sensor Arduino Library

[![Compile examples](https://github.com/Infineon/arduino-xensiv-dps3xx/actions/workflows/compile_examples.yml/badge.svg)](https://github.com/Infineon/arduino-xensiv-dps3xx/actions/workflows/compile_examples.yml)
[![Check links](https://github.com/Infineon/arduino-xensiv-dps3xx/actions/workflows/check_links.yml/badge.svg)](https://github.com/Infineon/arduino-xensiv-dps3xx/actions/workflows/check_links.yml)

Arduino library of Infineon's [**XENSIV™ Digital Pressure Sensors (DPS3xx)**](https://www.infineon.com/products/sensor/pressure-sensors/pressure-sensors-for-iot).

<img src="/docs/img/dps368.png" width=200>

## Supported Products

<table>
    <tr>
        <td rowspan=2>Products</td>
        <td><img src="docs/img/dps310.png" height="80"></td>
        <td><img src="docs/img/dps368.png" height="80"></td>
    </tr>
    <tr>
        <td style="text-align : center">XENSIV™ DPS310 *(deprecated)*</td>
        <td style="text-align : center"><a href="https://www.infineon.com/part/DPS368">XENSIV™ DPS368</a></td>
    </tr>
    <tr>
        <td rowspan=2>Shield2Go</td>
        <td><img src="docs/img/dps310-shield2go.png" height="80"></td>
        <td><img src="docs/img/dps368-shield2go.png" height="80"></td>
    </tr>
    <tr>
        <td style="text-align : center">XENSIV™ DPS310 Shield2Go *(deprecated)*</td>
        <td style="text-align : center"><a href="https://www.infineon.com/evaluation-board/S2GO-PRESSURE-DPS368">XENSIV™ DPS368 Shield2Go *(deprecated)*</a></td>
    </tr>
    <tr>
       <td rowspan=2>Kit 2Go</td>
        <td><img src="docs/img/dps310-kit2go.png" height="80"></td>
        <td><img src="docs/img/dps368-kit2go.png" height="80"></td>
    </tr>
    <tr>
        <td style="text-align : center">XENSIV™ DPS310 Kit 2Go *(deprecated)*</a></td>
        <td style="text-align : center"><a href="https://www.infineon.com/evaluation-board/KIT-DPS368-2GO">XENSIV™ DPS368 Kit 2Go</a></td>
    </tr>
</table>

## Getting Started

### Installation
To install this library in the Arduino IDE, please go to **Sketch** > **Include Library** > **Manage Libraries...** and search for the `XENSIV Digital Pressure Sensor` library by `Infineon Technologies` in the Arduino library manager.

### Usage
Please see the example sketches in the `/examples` directory in this repository to learn more about the usage of the library. Especially, take care of the respective SPI and I²C configuration of the sensor. You can find more information in the [Wiki](https://github.com/Infineon/arduino-xensiv-dps3xx/wiki).

## License
See the [LICENSE](LICENSE.md) file for more details.

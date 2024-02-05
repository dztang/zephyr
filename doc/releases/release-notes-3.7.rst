:orphan:

.. _zephyr_3.7:

Zephyr 3.7.0 (Working Draft)
############################

We are pleased to announce the release of Zephyr version 3.7.0.

Major enhancements with this release include:

* A new, completely overhauled hardware model has been introduced. This changes
  the way both SoCs and boards are named, defined and constructed in Zephyr.
  Additional information can be found in the :ref:`board_porting_guide`.

An overview of the changes required or recommended when migrating your application from Zephyr
v3.6.0 to Zephyr v3.7.0 can be found in the separate :ref:`migration guide<migration_3.7>`.

The following sections provide detailed lists of changes by component.

Security Vulnerability Related
******************************
The following CVEs are addressed by this release:

More detailed information can be found in:
https://docs.zephyrproject.org/latest/security/vulnerabilities.html

Architectures
*************

* ARC

* ARM

* Xtensa

Bluetooth
*********

Boards & SoC Support
********************

* Added support for these SoC series:

* Made these changes in other SoC series:

* Added support for these ARM boards:

* Added support for these Xtensa boards:

* Made these changes for ARM boards:

* Made these changes for RISC-V boards:

* Made these changes for native/POSIX boards:

* Added support for these following shields:

Build system and Infrastructure
*******************************

Drivers and Sensors
*******************

* ADC

* Auxiliary Display

* Audio

* Battery backed up RAM

* CAN

  * Deprecated the :c:func:`can_calc_prescaler` API function, as it allows for bitrate
    errors. Bitrate errors between nodes on the same network leads to them drifting apart after the
    start-of-frame (SOF) synchronization has taken place, leading to bus errors.
  * Extended support for automatic sample point location to also cover :c:func:`can_calc_timing` and
    :c:func:`can_calc_timing_data`.

* Clock control

* Counter

* Crypto

* Display

* DMA

* Entropy

* Ethernet

* Flash

* GNSS

* GPIO

* I2C

* I2S

* I3C

* IEEE 802.15.4

* Input

* MDIO

* MFD

* PCIE

* MEMC

* MIPI-DBI

* Pin control

* PWM

* Regulators

* Retained memory

* RTC

* SMBUS:

* SDHC

* Sensor

  * Replaced outdated :dtcompatible:`we,wsen-itds` accelerometer driver
    and renamed it to :dtcompatible:`we,wsen-itds-2533020201601`.
  * Replaced outdated :dtcompatible:`we,wsen-hids` humidity sensor driver
    and renamed it to :dtcompatible:`we,wsen-hids-2525020210001`.
  * Added Würth Elektronik HIDS-2525020210002
    :dtcompatible:`we,wsen-hids-2525020210002` humidity sensor driver.
  * Replaced outdated :dtcompatible:`we,wsen-pads` absolute pressure sensor driver
    and renamed it to :dtcompatible:`we,wsen-pads-2511020213301`.
  * Replaced outdated :dtcompatible:`we,wsen-pdus` differential pressure sensor driver
    and renamed it to :dtcompatible:`we,wsen-pdus-25131308XXXXX`.
  * Added Würth Elektronik ISDS-2536030320001
    :dtcompatible:`we,wsen-isds-2536030320001` 6-axis IMU sensor driver.
  * Replaced outdated :dtcompatible:`we,wsen-tids` temperature sensor driver
    and renamed it to :dtcompatible:`we,wsen-tids-2521020222501`.

* Serial

* SPI

* USB

* W1

* Wi-Fi

Networking
**********

* LwM2M:

  * Added new API function:

    * :c:func:`lwm2m_set_bulk`

USB
***

Devicetree
**********

Libraries / Subsystems
**********************

* Management

* Logging

* Modem modules

* Picolibc

* Power management

* Crypto

* Retention

* SD

* Storage

* POSIX API

* LoRa/LoRaWAN

* ZBus

HALs
****

* STM32

MCUboot
*******

zcbor
*****

LVGL
****

Tests and Samples
*****************

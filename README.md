# MSP430 FRAM Temperature Monitor

A Code Composer Studio implementation for the **MSP430FR4133**, combining its on-chip temperature sensor, persistent FRAM state and LED threshold indication.

## Features

- Read the temperature-sensor channel through the 10-bit ADC.
- Store the latest raw sample and a validity marker in persistent FRAM variables.
- Recover the stored value after startup and indicate a threshold condition on P1.0.
- Preserve the original CCS project, linker command file, target configuration and TI driverlib source.

`TEMP_THRESHOLD=630` is a raw ADC threshold, not a calibrated temperature in Celsius. This is a lab implementation, not a calibrated thermometer or an atomic power-fail logging system. The source is preserved without silently changing its sensor or FRAM behavior.

## Layout

| Path | Content |
|---|---|
| `ccs-project/` | Main MSP430FR4133 implementation and importable CCS project |
| `scripts/build.py` | Optional native TI compiler build, with an explicit compiler/device-header location |
| `reference/early-standalone/` | Earlier standalone source using different ADC registers; not the default FR4133 version |
| `reports/` | Original combined lab PDF/DOCX |
| `results/screenshots/` | Original FRAM/temperature screenshots extracted from the report |
| `provenance/` | Source hashes and checks actually performed during archiving |

## Open in Code Composer Studio

1. Install CCS with MSP430 support and the TI MSP430 compiler **21.6.x** (the recovered project used **21.6.1.LTS**).
2. Import `ccs-project/` as an existing CCS project. Its original internal project name is `MSP-430_NVM`, retained for compatibility.
3. Select **MSP430FR4133** and update local compiler/device-support paths if CCS asks.
4. Build the project. Debug/Release outputs and workspace caches are intentionally excluded.
5. On the intended LaunchPad, flash and verify ADC sampling, the LED threshold and FRAM retention using the original report as reference.

## Build without opening CCS

On Windows with the existing TI compiler/device headers installed:

```powershell
py scripts/build.py --compiler-dir "C:\ti\ccs2040\ccs\tools\compiler\ti-cgt-msp430_21.6.1.LTS" --device-include "C:\ti\ccs2040\ccs\ccs_base\msp430\include"
```

This compiles and links `build/MSP430_FRAMTempMonitor.out`; it never connects to or flashes a board. Paths are arguments, so the script does not depend on the old Windows user directory.

## Original behavior and evidence

`storedTemp` and `framValid` use TI's `PERSISTENT` pragma. The magic marker indicates a stored value, but the two writes are not transactional and no checksum is implemented. Device write protection and the CCS program loader's treatment of persistent sections matter when testing power loss. Preserve and inspect the linker configuration before drawing conclusions from a reflash test.

`reference/early-standalone/` is a historical draft; use `ccs-project/main.c` for the FR4133 implementation. The original report also contains a Linux driver exercise, archived separately in [LinuxKernel_AddDriver](https://github.com/Well-Legend/LinuxKernel_AddDriver).

TI driverlib and startup/linker files retain their original notices. Reports and screenshots have no newly granted blanket license. Validation results distinguish compile/link checks from physical hardware testing.

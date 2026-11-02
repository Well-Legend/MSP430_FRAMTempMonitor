#!/usr/bin/env python3
"""Compile/link the recovered CCS project; never flash hardware."""
import argparse
import json
import os
import pathlib
import subprocess

p=argparse.ArgumentParser(description=__doc__)
p.add_argument('--compiler-dir',required=True)
p.add_argument('--device-include',required=True)
a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
project=root/'ccs-project'
compiler=pathlib.Path(a.compiler_dir).resolve()
device=pathlib.Path(a.device_include).resolve()
cl=compiler/'bin'/('cl430.exe' if os.name=='nt' else 'cl430')
if not cl.is_file() or not (device/'msp430fr4133.h').is_file():p.error('TI compiler or MSP430FR4133 device header not found')
build=root/'build';build.mkdir(exist_ok=True)
driver=project/'driverlib/MSP430FR2xx_4xx'
flags=['-vmspx','--use_hw_mpy=none','--define=__MSP430FR4133__','--define=DEPRECATED','-g','--printf_support=minimal','--diag_warning=225','--diag_wrap=off','--display_error_number','--silicon_errata=CPU21','--silicon_errata=CPU22','--silicon_errata=CPU40']
includes=['--include_path='+str(x) for x in [device,project,driver,compiler/'include']]
sources=[project/'main.c',*sorted(driver.glob('*.c'))]
objects=[]
for source in sources:
    target=build/source.relative_to(project).parent;target.mkdir(parents=True,exist_ok=True)
    subprocess.run([str(cl),*flags,*includes,'-c','--obj_directory='+str(target),str(source)],cwd=build,check=True)
    obj=target/(source.stem+'.obj');assert obj.is_file(),str(obj)
    objects.append(obj)
output=build/'MSP430_FRAMTempMonitor.out'
subprocess.run([str(cl),*flags,'-z','--heap_size=160','--stack_size=160','--cinit_hold_wdt=on','--reread_libs','--warn_sections','--rom_model','--output_file='+str(output),'--map_file='+str(build/'MSP430_FRAMTempMonitor.map'),'--xml_link_info='+str(build/'MSP430_FRAMTempMonitor_linkInfo.xml'),*['-i'+str(x) for x in [device,compiler/'lib',compiler/'include']],*[str(x) for x in objects],str(project/'lnk_msp430fr4133.cmd'),'-llibc.a'],cwd=build,check=True)
print(json.dumps({'compiled_sources':len(sources),'output':str(output),'output_bytes':output.stat().st_size,'hardware_flashed':False}),flush=True)

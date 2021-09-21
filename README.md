<!-- HTML Comment -->

<!-- GitHub Markdown / HTML Format Tips

* Line Breaks in Text
  <br/> for single break

* Link with spaces in file path
  [Link Text](/folder/file%20path%20with%20spaces)

* Link to a Heading
  [Link Text](#link-to-a-multi-word-heading)

* File paths must use forward slashes '/' to create useable relative links in GitHub

* Side-by-side images on GitHub
  <p float="left">
  <img src="/img/pcb-001.jpg" alt="pcb-001.jpg" width="300">
  <img src="/img/pcb-002.jpg" alt="pcb-002.jpg" width="300">
  </p>

* Clickable Image URL link must have HTML code on SAME LINE, for example:
  <a href="docs/schematic.pdf"><img src="/img/schematic.jpg" alt="schematic.jpg" width="400"></a>

* Code Block Syntax Highlighting tags:
  http://www.rubycoloredglasses.com/2013/04/languages-supported-by-github-flavored-markdown/
  ```cpp

* Emojis: copy-paste directly into text https://emojipedia.org/

* Table Generator: https://www.tablesgenerator.com/markdown_tables
  | header A | header B | header C |
  |----------|----------|----------|
  | item 1   | item 2   | item 3   |
  |          |          |          |
-->

<!-- Project Title -->
# <img src="/img/icon.png" alt="icon.png" width="30"> Either-Or
**Client**: AAPlasma / Greg Fridman

<!-- Project Description -->
UV LED room-scale air purifier

<!-- Main Project Image -->
<img src="/img/hw001.jpg" alt="hw001.jpg" width="600">

---

### README Outline
* [**Documentation**](#documentation)
    * [User Documentation](#user-documentation)
    * [Developer Documentation](#developer-documentation)

<!--    * [System Block Diagram](#system-block-diagram) -->
<!--    * [Developer Resources](#developer-resources)-->
<!--    * [Production Documentation](#production-documentation)-->
* [**Hardware**](#hardware)
* [**Firmware**](#firmware)
* [**Software**](#software)
 ---

## <img src="/img/documentation_icon.png" alt="documentation_icon.png" width="20"> Documentation

<!--
### System Block Diagram
  <img src="/img/system_block_diagram.png" alt="system_block_diagram.png" width="700"><br><br>
-->

###  User Documentation
* <img src="/img/cli_icon.png" alt="cli_icon.png " width="15"> [**Command Line Interface Guide**](docs/command_line_interface_guide.md)

<!--
1. Initial Setup User Guide
1. Quick Start User Guide
1. Field User Guide
1. Technical User Manual
-->

### Developer Documentation

* <img src="/img/platformio_icon.png" alt="platformio_icon.png" width="15"> [**PlatformIO IDE Guide**](docs/platformio_ide_guide.md)

* <img src="/img/documentation_icon.png" alt="documentation_icon.png" width="15"> [**Firmware File Structure**](docs/firmware_file_structure.md)

* <img src="/img/folders.png" alt="folders.png" width="15"> [**Project Folder Structure**](/utils/print_folder_structure/folder_structure.md)
    > _**Note**_: to update, run [`print_folder_structure.bat`](/utils/print_folder_structure) from the locally cloned repo on a _Windows 10_ PC

<!-- 1. [**Example Firmware Setup Guide** ](docs/example_firmware_setup_guide.md) -->

<!--
### Developer Resources
-->

<!--
#### Production Documentation
1. Firmware Upload Procedure
1. Device Commissioning & Configuration Procedure
1. Production & Assembly Guide
1. Functional Verification Procedure
-->




---

## <img src="/img/hardware_icon.png" alt="hardware_icon.png" width="20"> Hardware

**Bill of Materials**: [Google Sheet](https://docs.google.com/spreadsheets/d/1uxv25DpGCV7qv-_Im_NRR5OFvUTFIbNF_wAqFNQ1yQ4/edit#gid=0) <!-- | [Simplified PDF](docs/bom-simplified.pdf)-->

**Photo Gallery**<br>
<img src="/img/hw001.jpg" alt="hw001.jpg" height="400"> <img src="/img/hw002.jpg" alt="hw002.jpg" height="400"><br><br>
<img src="/img/hw003.jpg" alt="hw003.jpg" height="400"> <img src="/img/hw004.jpg" alt="hw004.jpg" height="400"><br><br>


<!--
#### Electronics

* **Schematic**: [PDF](docs/schematic.pdf) <br>

  <a href="docs/schematic.pdf"><img src="/img/schematic.jpg" alt="schematic.jpg" width="400"></a>
<br><br>
* **PCB Design**: [Autodesk Eagle Files](hardware/elec/) <br>

  <p float="left">
  <img src="/img/pcb-render-top.png" alt="pcb-render-top.png" width="100">
  <img src="/img/pcb-render-bottom.png" alt="pcb-render-bottom.png" width="100">
  </p>
-->

<!--
#### Mechanical
* **3D Design**: [eDrawings Files](hardware/mech/eDrawings) <br>

  <a href="hardware/mech/eDrawings"><img src="/img/edrawing.jpg" alt="edrawing.jpg" width="400"></a>
-->

---

## <img src="/img/firmware_icon.png" alt="firmware_icon.png" width="25"> Firmware

**Source Code**: [`either-or_fw`](firmware/platformio_ide)

* **Target device**: [*Adafruit Feather M0 Express*](https://www.adafruit.com/product/3403)

* **IDE**: [PlatformIO IDE](https://platformio.org/)
* **Libraries**: see `lib_deps` section in [`platformio.ini`](firmware/platformio_ide)
<!--
  * **Protocol Stack**:
  * **Compiler Toolchain**:
-->

**Flow Diagram**:<br><br>
<img src="/img/firmware_flow_diagram.png" alt="firmware_flow_diagram.png" width="300">


---

<!--
## <img src="/img/software_icon.png" alt="software_icon.png" width="25"> Software

---
-->

*************
Release Notes
*************

Version 4.1.5
=============

* First public release.
* Configurable deposition by source.
* Stability improvements.
* Improved High-DPI support.
* Internal refactoring for MCABLE integration.
* Add moving averages to time series export function.

Version 4.1.4
=============

* Configurable flux profile by source.
* Interactive plotting of reference flux profile.
* Improved support for buffer zones.
* New command line post-processing utility (ncpost).
* Receptors are now set at the scenario level.
* Improved error handling when importing sources from AERMOD input files.

Version 4.1.3
=============

* Improved error handling for project files.
* Probabilistic flux profile assignment.
* Resample distributions in source table.
* Support for relative paths to SFC/PFL files.
* Fixed default receptor height.
* Project file drag-and-drop support.
* Set column visibility in source table.

Version 4.1.2
=============

* Improved error handling and friendlier error messages when importing sources from AERMOD input files.
* New discrete distribution editor for application start time.

Version 4.1.1
=============

* Fixed post-processing bug which prevented analysis of POSTFILES where 1 hour averaging period was not used.
* New receptor ring generation algorithm based on GEOS linear referencing.
* Logging of SOFEA-AERMOD errors via IPC, and new 'Model Output' window.

Version 4.1.0
=============

* Import AREAPOLY sources from GIS shapefiles.
* New meteorological data diagnostics utility to report calm/missing hours and generate wind roses.
* Edit receptor colors.
* Add support for dry and wet deposition.
* Edit averaging periods.
* New analysis module with unit conversion, additional metadata, selection of data subsets.
* New input file viewer based on Scintilla with AERMOD syntax highlighting.

Version 4.0.2
=============

* Improved logging support.
* Improved visual styles.
* Fixed several memory leaks.
* Fixed mass and flux calculations.

Version 4.0.1
=============

* Corrected input and flux file generation for multiple source groups.
* Fixed discrepancy between local and UTC time.

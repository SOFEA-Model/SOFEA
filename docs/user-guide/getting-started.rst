..  _getting_started:

Getting Started
===============

SOFEA implements a generic methodology to determine fumigant air 
concentrations in large and diverse air sheds. Fundamentally, SOFEA 
implements a pre-processor that sequentially develops complex AERMOD 
input files and a post-processor that uses high-performance algorithms 
to calculate summary statistics, moving averages, estimated percentiles, 
and empirical distributions. Simulations of virtually any complexity can 
be developed by composition of fundamental components: 

* **Scenario**: Defines global parameters that apply to the complete
  AERMOD input file. Includes general settings for the dispersion model,
  meteorological data, and flux profiles.
  
* **Source Group**: Defines Monte Carlo parameters and buffer zones
  that apply to a group of sources (treated fields). Corresponds to
  ``SO SRCGROUP`` in AERMOD.
  
* **Source**: Defines the geometry and emission parameters for an
  individual source term (treated field). Emission parameters can be
  defined for a specific source or derived from distributions specified
  at the Source Group level.

The input structure is hierarchical, with one scenario containing one or
more source groups, and one source group containing one or more sources.
One SOFEA project can contain multiple scenarios. Component dependencies
are illustrated in the following diagram:
::

     Project
     └──Scenario
        ├──Source Group
        │  ├──Source
        │  ├──Receptors
        └──Flux Profile

The SOFEA GUI provides a single integrated environment for defining 
scenarios, running the air dispersion model, and post-processing results. 
To open the GUI, double-click sofea.exe in the application folder. 

.. image:: ../_static/MainWindow.png
   :width: 624
   :align: center
   :alt: Main Window

The main window is divided into the Project Browser, Messages, and Workspace:

* **Project Browser**: Manages all of the scenarios and source groups in a project. Most options are accessed via the context menu displayed when right-clicking a scenario or source group.
* **Messages**: Captures informational messages and errors from SOFEA and AERMOD while running.
* **Workspace**: The central area of the window that displays AERMOD input files and the source data table.

The main project controls are available in the menus and toolbar:

.. epigraph::

    |NewScenario| **New Scenario**
        Creates a new scenario with default settings.
        
    |OpenProject| **Open Project**
        Opens an existing project file (.sofea).
        
    |SaveProject| **Save Project**
        Saves all scenario and source group data to the project file.

    |RunModel| **Run Model**
        Opens the run dialog to start and monitor progress of runs.

    |AnalyzeResults| **Analyze Results**
        Opens the post-processing utilities to analyze an output file.

.. |NewScenario| image:: ../_static/NewFile_16x.png
.. |OpenProject| image:: ../_static/OpenFolder_16x.png
.. |SaveProject| image:: ../_static/Save_16x.png
.. |RunModel| image:: ../_static/Run_16x.png
.. |AnalyzeResults| image:: ../_static/Measure_16x.png
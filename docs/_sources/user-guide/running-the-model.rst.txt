..  _running_the_model:

Running the Model
=================

Once the set-up of your project is complete, proceed to running the model by 
choosing |RunModel| **Run Model** through the main menu or by clicking a green 
button |RunModel| in the toolbar. This opens the run dialog box which allows 
you to start and monitor the progress of runs.

SOFEA will save all model outputs in subdirectories below the directory which
contains the project file, as shown at the top of the dialog box. Each subdirectory
has a unique name consisting of the Scenario name followed by a date and time
string showing the date and time when the model run started, up to a millisecond.

The run dialog box lists all **Scenarios** defined within the **Project Browser**.
You can choose to run a single Scenario, a certain set of Scenarios by picking
them with a help of the Shift key, or all Scenarios at once by clicking 
**Select All**.

At the bottom of the run dialog box, a text string indicates a number, *n*, 
of available processor cores on the computer you are using. This indicates the 
maximum number of Scenarios that can be run in parallel. In case you selected 
more than **n** scenarios to run, once you hit the **Run** button, the first *n*
scenarios from the list will be initiated, while other scenarios will be waiting
in a queue, and will follow as initial runs complete, *n* runs at a time. The
status of each run is shown in the run dialog box along with the date and time
when each run started, time elapsed since each run has started, and percentage of
the run completed. Each individual scenario run can be interrupted by clicking the
**Stop** button.

While AERMOD is running, any warning, error, or other informational messages from
AERMOD are dynamically displayed within the **Model Output** window of the SOFEA
GUI. Progress reporting is an extension to the regulatory version of AERMOD.

.. |RunModel| image:: ../_static/Run_16x.png
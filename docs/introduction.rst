************
Introduction
************

The development of a numerical modeling tool for the soil fumigant 
1,3-dichloropropene (1,3-D) started several decades ago using the 
`Industrial Source Complex Short Term (ISCST3)`_ air dispersion model. 
[#f1]_ Early work was extended by incorporating a soil fate modeling 
tool, the `Pesticide Root Zone Model (PRZM3)`_, to simulate the source 
strength used in ISCST3 air dispersion calculations. [#f2]_ This initial 
work was the forerunner of the SOil Fumigant Exposure Assessment (SOFEA) 
modeling system, a stochastic numerical modeling tool developed by 
Dow AgroSciences as a regulatory tool to evaluate and manage human 
inhalation exposure potential associated with the use of soil fumigants 
and other semi-volatile or volatile compounds. 

.. _Industrial Source Complex Short Term (ISCST3): https://www.epa.gov/scram/air-quality-dispersion-modeling-alternative-models#isc3
.. _Pesticide Root Zone Model (PRZM3): https://www.epa.gov/ceam/przm-version-index

The original SOFEA model was first described in 2005 [#f3]_ in 
addressing the need to model chemical concentrations in ambient air in 
basins spanning several California townships and assess long-term human 
inhalation exposures and potential human health risks associated with 
1,3-D. Originally a pre- and post-processor for the ISCST3 air 
dispersion model, SOFEA was updated in 2016 to incorporate the `AMS/EPA 
Regulatory Model (AERMOD)`_. SOFEA allows users to probabilistically 
generate input files using over 20 discrete and continuous probability 
distributions for input parameters, and it provides graphical tools for 
analyzing weather data, creating flux profiles, importing source 
geometry from Geographic Information System (GIS) data, generating receptors 
using computational geometry methods, and high-performance post-processing
of results using the NetCDF scientific data format incorporated directly
into AERMOD.

.. _AMS/EPA Regulatory Model (AERMOD): https://www.epa.gov/scram/air-quality-dispersion-modeling-preferred-and-recommended-models#aermod

SOFEA has exceptional attributes and functionality compared to other 
similar modeling tools for addressing the exposure and risk from the use 
of volatile (or semi-volatile) pesticides. In a collaborative effort 
between Dow AgroSciences and `Exponent, Inc.`_ (an Engineering and 
Scientific Consulting company), SOFEA was upgraded to modern software 
engineering standards, and a new graphical interface was developed with 
C++ and Qt to provide users an Integrated Development Environment 
(IDE)-like experience in creating new simulation projects. Summary 
statistics, moving averages, and quantiles can be calculated efficiently 
over millions of data points, comprising hourly concentrations over 
several years and thousands of receptors.

.. _Exponent, Inc.: https://www.exponent.com/

Multiple publications using SOFEA have been documented since SOFEA was
first developed. [#f4]_ [#f5]_ [#f6]_ [#f7]_ [#f8]_

.. rubric:: References

.. [#f1] Cryer SA, van Wesenbeeck IJ. Predicted 1,3-dichloropropene air concentrations resulting from tree and vine applications in California. Journal of Environmental Quality. 2001;20:1887-1895
.. [#f2] Cryer SA, van Wesenbeeck IJ, Knuteson JA. Predicting regional emissions and near-field air concentrations of soil fumigants using modest numerical algorithms: A case study using 1,3-dichloropropene. Journal of Agricultural and Food Chemistry. 2003;51:3401-3409
.. [#f3] Cryer SA. Predicting soil fumigant acute, sub-chronic, and chronic air concentrations under diverse agronomic practices. Journal of Environmental Quality. 2005;34:2197-2207 
.. [#f4] de Cirugeda Helle O, van Wesenbeeck IJ, Cryer SA. SOFEA modeling of 1,3-Dichloropropene concentrations in ambient air in high fumigant use areas of the United States. American Chemical Society (ACS). 255th National meeting. Boston, MA. August 19-23, 2018 
.. [#f5] USEPA Agency Scientific Advisory Panel 2004. Fumigant Bystander Exposure Model Review: Soil Fumigant Exposure Assessment System (SOFEA) Using Telone as a Case Study. Available from: https://archive.epa.gov/scipoly/ sap/meetings/web/html/090904_mtg. html [Accessed: September 10, 2004]
.. [#f6] Cryer SA, van Wesenbeeck IJ. Coupling field observations, soil modeling, and air dispersion algorithms to estimate 1,3-dichloropropene and chloropicrin flux and exposure. Journal of Environmental Quality. 2011;2011(40):1450-1461. (Invited Paper for Special Submissions: Agricultural Air Quality)
.. [#f7] van Wesenbeeck IJ, Cryer SA, de Cirugeda Helle O, Li C, Driver J. Comparison of regional air dispersion simulation and ambient air monitoring data for the soil fumigant 1,3-dichloropropene. Science of the Total Environment. 2016:569-570 603-610
.. [#f8] van Wesenbeeck IJ, Cryer SA, Havens PL, Houtman BA. Use of SOFEA to predict 1,3-D concentrations in air in high use regions of California. Journal of Environmental Quality. 2011;40:1462-1469






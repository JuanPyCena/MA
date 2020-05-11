# Testbench documentation

The Testbench can be used to verify the performance of the IMM prototype. 
To do so test data from a csv files can be loaded. This is then used as measurement data for the IMM.
The test data must contain time information, which is crucial for the IMM since some
matrices are depended on the time interval since the last measurement.

The results of the IMM are saved into a csv file and therefore persistent. This file is read in and plotted. 
Concretely, the measurement (test data) is plotted, as a reference, and the calculated IMM data. 
The data are plotted x-Coordinated over y-Coordinated, therefore representing movement in a 2D space.
Additionally the probabilities of the individual IMM modes are plotted over time, as well as the error of the position, velocity and 
acceleration over time.

How to use the Testbench:

* Create test data using the TestDataFactory. How to use the factory is described within the class itself.
* Read in a created test data using the DataFileInterface.read_test_data(). This function needs a string
representation of the path to the test data csv file. The test data is then saved into a dictionary.
* Create a TestbenchIMM object using the test data dictionary and a path where the IMM calculated data should be saved to.
* Call the 'run()' function of the TestbenchIMM.,
* Create a TestbenchPlotter object using the IMM data file and the test data file.
* Call either the 'plot_test_data()' function, to plot only the test data, 
the 'plot_imm_data()' function, to plot only the imm calculated data with all mode probabilities and errors, 
or the 'plot_combined()' function, to plot the test data + the imm calculated data to have the visually overlap each other.
# IMM Configuartion Concept

### The idea and motivation
The basic idea of the configuration concept of the IMM is very simple.
It should be possible to split the logical and mathematical part between 
the code and the configuartion.

To achieve this all matrices needed for the calculations should be defined by 
the configuration. The IMM loads those matrices on start up and uses them to 
perform all calculations defined by its logic.

Using this allows an easy adaption of the IMM for other needs and feeds perfectly 
into a product focused development

### Configurable Parameters 
All formulas and the mathematical symbols are based on this [paper](https://drive.google.com/open?id=1KRITwuqHBTCtndpCvFQknt3VB0lFSruw).

In general it should be possible to to configure what kind of filters are to be used.
The IMM should read from a list which kind of filters it should use.  
The list could look like this:

*filters = [KF, KF, EKF, EKF]*

For each filter the following parameters should be configurable
- The system transition matrix (**F**), this matrix can consist of other variables aswell to provid the possibility to configure the system easily for new aiports
- The input control matrix (**G/B**, _optional_), this matrix allows to manipulate the state directly using the input. [This can be abused to allow for forcefully extrapolating the state](https://en.wikipedia.org/wiki/Kalman_filter#Example_application,_technical) 
- The process noise (**Q**, _optional_), this matrix can consist of other variables aswell, like *dt* (which would be a variable within the code)
- The covariance matrix (**P**, _optional_), this matrix should provide an initial covariance state
- The measurement control matrix (**H**), this matrix is used to project the measurement vector into the state space
- The measurement uncertainty matrix (**R**, _optional_), this matrix can consist of other variables aswell, like *dt* (which would be a variable within the code),
  it can be used to directly control the update of the state
  
For the IMM the following parameters should be configurable:
- The state(**x**), what kind of variables the system expects (e.g.: [pos_x, pos_y, vel_x, vel_y])
- The initial mode probabilites (**u**, _optional_), usually in the beginning all modes are equally likely, but it should be configurable for completeness 
- The markov transition matrix (**P**), this matrix defines the transitions between the modes/filters of the IMM. The sum of each column and each row must be 1
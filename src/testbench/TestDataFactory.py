from src.utils.DataFileInterface import DataFilteInterface as FileReadWrite
from src.utils.Decorators import *
import numpy as np

"""
How to use this class:
Create a static function 'test_setX()'. This function uses lambda/nested time depended functions to represent movement,
velocity and acceleration of the test data. The veloctiy and acceleration must be analytical derivatives of the position.
To add noise into the data additive gaussion noise can be added manually within the function 'test_setX()'.

The function then must call the function 'create_test_data', or a similar function, to actually create the csv file for the test data.

Using the 'main' of this class the 'test_setX()' function may be called.
"""

# Class only serves as namespace, all functions are static
class TestDataFactory(object):

    ##############################################################################
    @staticmethod
    def create_test_data(time, pos_functions, vel_functions, acc_functions, test_file):
        """
        Evaluates the positions, velocity and accelartion functions by a given time vector and saves it to a given test_file
        :param time: np.ndarray -  holds an equidistant time vector.
        :param pos_functions: list - list of numpy functions which can be evaluated by time to calculate the position
        :param vel_functions: list - list of numpy functions which can be evaluated by time to calculate the position, must be analytical derivative of position
        :param acc_functions: list - list of numpy functions which can be evaluated by time to calculate the position, must be analytical derivative of velocity
        :param test_file: str - path to the test file which shall be written
        """
        times, positions, velocities, accelerations = [], [], [], []
        for t in time:
            # round to 4 digits after comma
            t = round(t, 4)
            pos = [fun(t) for fun in pos_functions]
            vel = [fun(t) for fun in vel_functions]
            acc = [fun(t) for fun in acc_functions]

            times.append(t)
            positions.append(np.array(pos))
            velocities.append(np.array(vel))
            accelerations.append(np.array(acc))

        test_data_dict                  = dict()
        test_data_dict["time"]          = times
        test_data_dict["position"]      = positions
        test_data_dict["velocity"]      = velocities
        test_data_dict["acceleration"]  = accelerations

        FileReadWrite.write_test_data(test_data_dict, test_file)

    ##############################################################################
    @staticmethod
    def create_noisy_test_data(time, pos_functions, vel_functions, acc_functions, test_file):
        """
        Evaluates the positions, velocity and accelartion functions by a given time vector and saves it to a given test_file
        :param time: np.ndarray -  holds an equidistant time vector.
        :param pos_functions: list - list of numpy functions which can be evaluated by time to calculate the position
        :param vel_functions: list - list of numpy functions which can be evaluated by time to calculate the position, must be analytical derivative of position
        :param acc_functions: list - list of numpy functions which can be evaluated by time to calculate the position, must be analytical derivative of velocity
        :param test_file: str - path to the test file which shall be written
        """
        times, positions, velocities, accelerations = [], [], [], []
        for t in time:
            # round to 4 digits after comma
            t = round(t, 4)
            pos = [fun(t) + np.random.rand() for fun in pos_functions]
            vel = [fun(t) + np.random.rand()*0.1 for fun in vel_functions]
            acc = [fun(t) + np.random.rand()*0.01 for fun in acc_functions]

            times.append(t)
            positions.append(np.array(pos))
            velocities.append(np.array(vel))
            accelerations.append(np.array(acc))

        test_data_dict                  = dict()
        test_data_dict["time"]          = times
        test_data_dict["position"]      = positions
        test_data_dict["velocity"]      = velocities
        test_data_dict["acceleration"]  = accelerations

        FileReadWrite.write_test_data(test_data_dict, test_file)

    ##############################################################################
    @staticmethod
    def create_combined_test_data(time, pos_functions, vel_functions, acc_functions, test_file):
        """
        Evaluates the positions, velocity and accelartion functions by a given time vector and saves it to a given test_file
        :param time: np.ndarray -  holds an equidistant time vector.
        :param pos_functions: list - list of numpy functions which can be evaluated by time to calculate the position
        :param vel_functions: list - list of numpy functions which can be evaluated by time to calculate the position, must be analytical derivative of position
        :param acc_functions: list - list of numpy functions which can be evaluated by time to calculate the position, must be analytical derivative of velocity
        :param test_file: str - path to the test file which shall be written
        """
        times, positions, velocities, accelerations = [], [], [], []
        for t in time:
            # round to 4 digits after comma
            t = round(t, 4)
            if positions != []:
                last_pos = positions[len(positions)-1]
                pos = [fun(t, last_pos[idx]) for idx, fun in enumerate(pos_functions)]
            else:
                pos = [fun(t, 0) for fun in pos_functions]

            vel = [fun(t) for fun in vel_functions]
            acc = [fun(t) for fun in acc_functions]

            times.append(t)
            positions.append(np.array(pos))
            velocities.append(np.array(vel))
            accelerations.append(np.array(acc))

        test_data_dict                  = dict()
        test_data_dict["time"]          = times
        test_data_dict["position"]      = positions
        test_data_dict["velocity"]      = velocities
        test_data_dict["acceleration"]  = accelerations

        FileReadWrite.write_test_data(test_data_dict, test_file)

    ##############################################################################

    @staticmethod
    def test_set1():
        """
        Creates a test set using linear uniform motion.
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion.csv"
        time = np.linspace(0, 300, num=3001)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: t
        fun_pos_y = lambda t: t
        fun_vel_x = lambda t: 1
        fun_vel_y = lambda t: 1
        fun_acc_x = lambda t: 0
        fun_acc_y = lambda t: 0

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set2():
        """
        Creates a test set using constant linear acceleration motion
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_linear_acceleration_motion.csv"
        time = np.linspace(0, 300, num=3001)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: (t*t)/2
        fun_pos_y = lambda t: (t*t)/2
        fun_vel_x = lambda t: t
        fun_vel_y = lambda t: t
        fun_acc_x = lambda t: 1
        fun_acc_y = lambda t: 1

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set3():
        """
        Creates a test set using constant turning motion
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_turning_motion.csv"
        time = np.linspace(0, 300, num=3001)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: np.cos(0.1*t)
        fun_pos_y = lambda t: np.sin(0.1*t)
        fun_vel_x = lambda t: -0.1*np.sin(0.1*t)
        fun_vel_y = lambda t: 0.1*np.cos(0.1*t)
        fun_acc_x = lambda t: -0.1*0.1*np.cos(0.1*t)
        fun_acc_y = lambda t: -0.1*0.1*np.sin(0.1*t)

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set4():
        # Todo: Fix this, numbers are not realistic!!!
        """
        Creates a test set using combined motion, uniform linear -> constant turning -> constant accelaration
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_combined_motion.csv"
        time = np.linspace(0, 125, num=1251)

        # functions to be evalutated by time
        def fun_pos_x(t, old_pos):
            if t < 50:
                return t
            elif t >=50 and t < 120:
                t -= 50
                return np.cos(0.1*t) + old_pos
            else:
                t -= 120
                return (t*t)/2 + old_pos

        def fun_pos_y(t, old_pos):
            if t < 50:
                return t
            elif t >=50 and t < 120:
                t -= 50
                return np.sin(0.1*t) + old_pos
            else:
                t -= 120
                return (t*t)/2 + old_pos

        def fun_vel_x(t):
            if t < 50:
                return 1
            elif t >=50 and t < 120:
                t -= 50
                return -0.1*np.sin(0.1*t)
            else:
                t -= 120
                return t

        def fun_vel_y(t):
            if t < 50:
                return 1
            elif t >= 50 and t < 120:
                t -= 50
                return 0.1*np.cos(0.1*t)
            else:
                t -= 120
                return t

        def fun_acc_x(t):
            if t < 50:
                return 0
            elif t >=50 and t < 120:
                t -= 50
                return -0.1*0.1*np.cos(0.1*t)
            else:
                t -= 120
                return 1

        def fun_acc_y(t):
            if t < 50:
                return 0
            elif t >= 50 and t < 120:
                t -= 50
                return -0.1*0.1*np.sin(0.1*t)
            else:
                t -= 120
                return 1

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_combined_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set5():
        """
        Creates a test set using linear uniform motion.
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion_noise.csv"
        time = np.linspace(0, 300, num=3001)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: t
        fun_pos_y = lambda t: t
        fun_vel_x = lambda t: 1
        fun_vel_y = lambda t: 1
        fun_acc_x = lambda t: 0
        fun_acc_y = lambda t: 0

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_noisy_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set6():
        """
        Creates a test set using constant linear acceleration motion
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_linear_acceleration_motion_noise.csv"
        time = np.linspace(0, 300, num=3001)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: (t*t)/2
        fun_pos_y = lambda t: (t*t)/2
        fun_vel_x = lambda t: t
        fun_vel_y = lambda t: t
        fun_acc_x = lambda t: 1
        fun_acc_y = lambda t: 1

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_noisy_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set7():
        """
        Creates a test set using constant turning motion
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_turning_motion_noise.csv"
        time = np.linspace(0, 300, num=3001)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: np.cos(0.1*t)
        fun_pos_y = lambda t: np.sin(0.1*t)
        fun_vel_x = lambda t: -0.1*np.sin(0.1*t)
        fun_vel_y = lambda t: 0.1*np.cos(0.1*t)
        fun_acc_x = lambda t: -0.1*0.1*np.cos(0.1*t)
        fun_acc_y = lambda t: -0.1*0.1*np.sin(0.1*t)

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_noisy_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set8():
        """
        Creates a test set using linear uniform motion.
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion_non_uniform_time.csv"
        num = 3001
        time = np.linspace(0, 300, num=num)
        indices = range(num)
        remove_indices =  np.random.choice(indices, num // 4)

        time_removed = []
        for idx, step in enumerate(time):
            if idx not in remove_indices:
                time_removed.append(step)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: t
        fun_pos_y = lambda t: t
        fun_vel_x = lambda t: 1
        fun_vel_y = lambda t: 1
        fun_acc_x = lambda t: 0
        fun_acc_y = lambda t: 0

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_test_data(time_removed, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set9():
        """
        Creates a test set using constant linear acceleration motion
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_linear_acceleration_motion_non_uniform_time.csv"
        num = 3001
        time = np.linspace(0, 300, num=num)
        indices = range(num)
        remove_indices = np.random.choice(indices, num // 4)

        time_removed = []
        for idx, step in enumerate(time):
            if idx not in remove_indices:
                time_removed.append(step)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: (t * t) / 2
        fun_pos_y = lambda t: (t * t) / 2
        fun_vel_x = lambda t: t
        fun_vel_y = lambda t: t
        fun_acc_x = lambda t: 1
        fun_acc_y = lambda t: 1

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_test_data(time_removed, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

    @staticmethod
    def test_set10():
        """
        Creates a test set using constant turning motion
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_turning_motion_non_uniform_time.csv"
        num = 3001
        time = np.linspace(0, 300, num=num)
        indices = range(num)
        remove_indices = np.random.choice(indices, num // 4)

        time_removed = []
        for idx, step in enumerate(time):
            if idx not in remove_indices:
                time_removed.append(step)

        # lambda functions to be evalutated by time
        fun_pos_x = lambda t: np.cos(0.1 * t)
        fun_pos_y = lambda t: np.sin(0.1 * t)
        fun_vel_x = lambda t: -0.1 * np.sin(0.1 * t)
        fun_vel_y = lambda t: 0.1 * np.cos(0.1 * t)
        fun_acc_x = lambda t: -0.1 * 0.1 * np.cos(0.1 * t)
        fun_acc_y = lambda t: -0.1 * 0.1 * np.sin(0.1 * t)

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_test_data(time_removed, pos_functions, vel_functions, acc_functions, test_file)
    ##############################################################################

    @staticmethod
    def test_set11():
        """
        Creates a test set using combined motion, uniform linear -> constant turning -> constant accelaration
        """
        test_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv"
        time = np.linspace(0, 60, num=6001)

        # functions to be evalutated by time
        def fun_pos_x(t, old_pos):
            if t < 1.1:
                return (t * t) / 2
            elif t > 51:
                return old_pos
            elif t > 50:
                t -= 50
                return -(t * t) / 2 + old_pos
            else:
                t -= 1.1
                return t

        def fun_pos_y(t, old_pos):
            if t < 1.1:
                return (t * t) / 2
            elif t > 51:
                return old_pos
            elif t > 50:
                t -= 50
                return -(t * t) / 2  + old_pos
            else:
                t -= 1.1
                return t

        def fun_vel_x(t):
            if t < 1.1:
                return t
            elif t > 51:
                return 0
            elif t > 50:
                t -= 50
                return -t
            else:
                t -= 1.1
                return 1

        def fun_vel_y(t):
            if t < 1.1:
                return t
            elif t > 51:
                return 0
            elif t > 50:
                t -= 50
                return -t
            else:
                t -= 1.1
                return 1

        def fun_acc_x(t):
            if t < 1.1:
                return 1
            elif t > 51:
                return 0
            elif t > 50:
                return -1
            else:
                return 0

        def fun_acc_y(t):
            if t < 1.1:
                return 1
            elif t > 51:
                return 0
            elif t > 50:
                return -1
            else:
                return 0

        pos_functions = [fun_pos_x, fun_pos_y]
        vel_functions = [fun_vel_x, fun_vel_y]
        acc_functions = [fun_acc_x, fun_acc_y]

        # create actual dataset using the given lambda functions.
        TestDataFactory.create_combined_test_data(time, pos_functions, vel_functions, acc_functions, test_file)

    ##############################################################################

def main():
    TestDataFactory.test_set1()
    TestDataFactory.test_set2()
    TestDataFactory.test_set3()
    TestDataFactory.test_set4()
    TestDataFactory.test_set5()
    TestDataFactory.test_set6()
    TestDataFactory.test_set7()
    TestDataFactory.test_set8()
    TestDataFactory.test_set9()
    TestDataFactory.test_set10()
    TestDataFactory.test_set11()

if __name__ == "__main__":
    main()

#EOF
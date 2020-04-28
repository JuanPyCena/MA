import numpy as np
import matplotlib.pyplot as plt

from src.utils.DataFileInterface import DataFilteInterface as DFI
from src.utils.Decorators import *

class TestbenchPlotter(object):

    @typecheck(str, str)
    def __init__(self, test_data_file, imm_data_file):
        # Determine path were figures should be saved to
        _, self.plot_name_test_data = test_data_file.rsplit("\\", 1)
        _, self.plot_name_imm_data = imm_data_file.rsplit("\\", 1)
        self.plot_name_test_data = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\plots\\" + self.plot_name_test_data[:-4] + "_test_data_plot"
        self.plot_name_imm_data = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\plots\\" + self.plot_name_imm_data[:-4]  + "_imm_data_plot"

        # Read test data
        self.test_data = DFI.read_test_data(test_data_file)

        # self.imm_data = dict()
        # self.df_interface = DFI(imm_data_file)
        # self.df_interface.read()
        # self.__retrieve_data()

    def __retrieve_data(self):
        # measurement_data
        measurement_data = self.df_interface.measurement_data()
        position_measurement, velocity_measurement, acceleration_measurement = [], [], []

        for data_point in measurement_data:
            pos, vel, acc = [], [], []
            pos.append(data_point[0])
            pos.append(data_point[1])
            vel.append(data_point[2])
            vel.append(data_point[3])
            acc.append(data_point[4])
            acc.append(data_point[5])
            position_measurement.append(pos)
            velocity_measurement.append(vel)
            acceleration_measurement.append(acc)

        self.imm_data["position_measurement"]     = position_measurement
        self.imm_data["velocity_measurement"]     = velocity_measurement
        self.imm_data["acceleration_measurement"] = acceleration_measurement

        # state_data
        state_data = self.df_interface.state_data()
        position_state, velocity_state, acceleration_state = [], [], []

        for data_point in state_data:
            pos, vel, acc = [], [], []
            pos.append(data_point[0])
            pos.append(data_point[1])
            vel.append(data_point[2])
            vel.append(data_point[3])
            acc.append(data_point[4])
            acc.append(data_point[5])
            position_state.append(pos)
            velocity_state.append(vel)
            acceleration_state.append(acc)

        self.imm_data["position_state"]     = position_state
        self.imm_data["velocity_state"]     = velocity_state
        self.imm_data["acceleration_state"] = acceleration_state

        # state_errors
        state_errors = self.df_interface.state_errors()
        position_errors, velocity_errors, acceleration_errors = [], [], []

        for data_point in state_data:
            pos, vel, acc = [], [], []
            pos.append(data_point[0])
            pos.append(data_point[1])
            vel.append(data_point[2])
            vel.append(data_point[3])
            acc.append(data_point[4])
            acc.append(data_point[5])
            position_errors.append(pos)
            velocity_errors.append(vel)
            acceleration_errors.append(acc)

        self.imm_data["position_errors"]     = position_errors
        self.imm_data["velocity_errors"]     = velocity_errors
        self.imm_data["acceleration_errors"] = acceleration_errors

        # mode_probabilities
        self.imm_data["mode_probabilities"] = self.df_interface.mode_probabilities()

    ##############################################################################

    def plot_test_data(self):
        x_pos = [pos[0] for pos in self.test_data["position"]]
        y_pos = [pos[1] for pos in self.test_data["position"]]
        x_vel = [vel[0] for vel in self.test_data["velocity"]]
        y_vel = [vel[1] for vel in self.test_data["velocity"]]
        x_acc = [acc[0] for acc in self.test_data["acceleration"]]
        y_acc = [acc[1] for acc in self.test_data["acceleration"]]

        fig, axs = plt.subplots(2, 2)

        axs[0, 0].plot(x_pos, y_pos)
        axs[0, 0].set_title('test_data_position')

        axs[1, 0].plot(x_vel, y_vel)
        axs[1, 0].set_title('test_data_velocity')

        axs[1, 1].plot(x_acc, y_acc)
        axs[1, 1].set_title('test_data_acceleration')
        fig.tight_layout()
        plt.savefig(self.plot_name_test_data, dpi=1200)
        plt.show()

    ##############################################################################

    def plot_imm_data(self):
        pass

    ##############################################################################

    def plot_combined(self):
        pass

# EOF
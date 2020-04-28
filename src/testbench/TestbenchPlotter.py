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
        self.plot_name_combined_data = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\plots\\" + self.plot_name_imm_data[:-4]  + "_combined_plot"

        # Read test data
        self.test_data = DFI.read_test_data(test_data_file)

        self.imm_data = dict()
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
        x_pos_measurement = [pos[0] for pos in self.imm_data["position_measurement"]]
        y_pos_measurement = [pos[1] for pos in self.imm_data["position_measurement"]]
        x_vel_measurement = [vel[0] for vel in self.imm_data["velocity_measurement"]]
        y_vel_measurement = [vel[1] for vel in self.imm_data["velocity_measurement"]]
        x_acc_measurement = [acc[0] for acc in self.imm_data["acceleration_measurement"]]
        y_acc_measurement = [acc[1] for acc in self.imm_data["acceleration_measurement"]]
        x_pos_state       = [pos[0] for pos in self.imm_data["position_state"]]
        y_pos_state       = [pos[1] for pos in self.imm_data["position_state"]]
        x_vel_state       = [vel[0] for vel in self.imm_data["velocity_state"]]
        y_vel_state       = [vel[1] for vel in self.imm_data["velocity_state"]]
        x_acc_state       = [acc[0] for acc in self.imm_data["acceleration_state"]]
        y_acc_state       = [acc[1] for acc in self.imm_data["acceleration_state"]]
        x_pos_error       = [pos[0] for pos in self.imm_data["position_error"]]
        y_pos_error       = [pos[1] for pos in self.imm_data["position_error"]]
        x_vel_error       = [vel[0] for vel in self.imm_data["velocity_error"]]
        y_vel_error       = [vel[1] for vel in self.imm_data["velocity_error"]]
        x_acc_error       = [acc[0] for acc in self.imm_data["acceleration_error"]]
        y_acc_error       = [acc[1] for acc in self.imm_data["acceleration_error"]]
        mode_prob_1       = [prob[0] for prob in self.imm_data["mode_probabilities"]]
        mode_prob_2       = [prob[1] for prob in self.imm_data["mode_probabilities"]]
        mode_prob_3       = [prob[2] for prob in self.imm_data["mode_probabilities"]]

        fig, axs = plt.subplots(4, 2)

        axs[0, 0].plot(x_pos_measurement, y_pos_measurement)
        axs[0, 0].set_title('measurement_position')

        axs[0, 1].plot(x_vel_measurement, y_vel_measurement)
        axs[0, 1].set_title('measurement_velocity')

        axs[0, 2].plot(x_acc_measurement, y_acc_measurement)
        axs[0, 2].set_title('measurement_acceleration')

        axs[0, 3].plot(x_acc_measurement, y_acc_measurement)
        axs[0, 3].set_title('measurement_acceleration')

        axs[0, 4].plot(self.test_data["time"], mode_prob_1, label="Mode1")
        axs[0, 4].plot(self.test_data["time"], mode_prob_2, label="Mode2")
        axs[0, 4].plot(self.test_data["time"], mode_prob_2, label="Mode3")
        axs[0, 4].set_title('mode_probabilities')
        axs[0, 4].legend()

        axs[1, 0].plot(x_pos_state, y_pos_state)
        axs[1, 0].set_title('state_position')

        axs[1, 1].plot(x_vel_state, y_vel_state)
        axs[1, 1].set_title('state_velocity')

        axs[1, 2].plot(x_acc_state, y_acc_state)
        axs[1, 2].set_title('state_acceleration')

        axs[1, 3].plot(x_acc_state, y_acc_state)
        axs[1, 3].set_title('state_acceleration')

        axs[1, 4].plot(x_pos_error, y_pos_error, label="Error Position")
        axs[1, 4].plot(x_vel_error, y_vel_error, label="Error Velocity")
        axs[1, 4].plot(x_acc_error, y_acc_error, label="Error Acceleration")
        axs[1, 4].set_title('state_errors')
        axs[1, 4].legend()

        fig.tight_layout()
        plt.savefig(self.plot_name_imm_data, dpi=1200)
        plt.show()

    ##############################################################################

    def plot_combined(self):
        x_pos_test_data = [pos[0] for pos in self.test_data["position"]]
        y_pos_test_data = [pos[1] for pos in self.test_data["position"]]
        x_vel_test_data = [vel[0] for vel in self.test_data["velocity"]]
        y_vel_test_data = [vel[1] for vel in self.test_data["velocity"]]
        x_acc_test_data = [acc[0] for acc in self.test_data["acceleration"]]
        y_acc_test_data = [acc[1] for acc in self.test_data["acceleration"]]
        x_pos_imm_state = [pos[0] for pos in self.imm_data["position_state"]]
        y_pos_imm_state = [pos[1] for pos in self.imm_data["position_state"]]
        x_vel_imm_state = [vel[0] for vel in self.imm_data["velocity_state"]]
        y_vel_imm_state = [vel[1] for vel in self.imm_data["velocity_state"]]
        x_acc_imm_state = [acc[0] for acc in self.imm_data["acceleration_state"]]
        y_acc_imm_state = [acc[1] for acc in self.imm_data["acceleration_state"]]
        x_pos_error = [pos[0] for pos in self.imm_data["position_error"]]
        y_pos_error = [pos[1] for pos in self.imm_data["position_error"]]
        x_vel_error = [vel[0] for vel in self.imm_data["velocity_error"]]
        y_vel_error = [vel[1] for vel in self.imm_data["velocity_error"]]
        x_acc_error = [acc[0] for acc in self.imm_data["acceleration_error"]]
        y_acc_error = [acc[1] for acc in self.imm_data["acceleration_error"]]

        fig, axs = plt.subplots(2, 2)

        axs[0, 0].plot(x_pos_test_data, y_pos_test_data, label="Position test data")
        axs[0, 0].plot(x_pos_imm_state, y_pos_imm_state, label="Position imm state")
        axs[0, 0].set_title('Position')

        axs[0, 1].plot(x_vel_test_data, y_vel_test_data, label="Velocity test data")
        axs[0, 1].plot(x_vel_imm_state, y_vel_imm_state, label="Velocity imm state")
        axs[0, 1].set_title('Velocity')

        axs[1, 0].plot(x_acc_test_data, y_acc_test_data, label="Acceleration test data")
        axs[1, 0].plot(x_acc_imm_state, y_acc_imm_state, label="Acceleration imm state")
        axs[1, 0].set_title('Acceleration')

        axs[1, 1].plot(x_pos_error, y_pos_error, label="Error Position")
        axs[1, 1].plot(x_vel_error, y_vel_error, label="Error Velocity")
        axs[1, 1].plot(x_acc_error, y_acc_error, label="Error Acceleration")
        axs[1, 1].set_title('Errors')
        axs[1, 1].legend()

        fig.tight_layout()
        plt.savefig(self.plot_name_combined_data, dpi=1200)
        plt.show()

# EOF
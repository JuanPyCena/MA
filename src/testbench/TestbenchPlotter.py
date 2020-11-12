import os
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
        subfolder_name = self.plot_name_imm_data[:-4]
        _, _, subfolder_name = subfolder_name.split("_", 2)
        try:
            os.mkdir("D:\\programming\\masterarbeit\\src\\testbench\\plots\\" + subfolder_name)
        except:
            pass
        self.plot_name_combined_data = "D:\\programming\\masterarbeit\\src\\testbench\\plots\\" + subfolder_name + "\\" + self.plot_name_imm_data[:-4]  + "_combined_plot"
        self.plot_name_test_data = "D:\\programming\\masterarbeit\\src\\testbench\\plots\\" + subfolder_name + "\\" + self.plot_name_test_data[:-4] + "_test_data_plot"
        self.plot_name_imm_data = "D:\\programming\\masterarbeit\\src\\testbench\\plots\\" + subfolder_name + "\\" + self.plot_name_imm_data[:-4]  + "_imm_data_plot"

        # Read test data
        self.test_data = DFI.read_test_data(test_data_file)

        self.imm_data = dict()
        self.df_interface = DFI(imm_data_file)
        self.df_interface.read()
        self.__retrieve_data()

    ##############################################################################

    def __retrieve_data(self):
        # measurement_data
        measurement_data = self.df_interface.z0
        position_measurement, velocity_measurement, acceleration_measurement = [], [], []
        pos_error = []

        for idx, data_point in enumerate(measurement_data):
            pos, vel, acc = [], [], []
            error = []
            pos.append(data_point[0])
            pos.append(data_point[1])
            # acc.append(data_point[2])
            # pos.append(data_point[3])
            # vel.append(data_point[4])
            # acc.append(data_point[5])
            position_measurement.append(pos)
            # velocity_measurement.append(vel)
            # acceleration_measurement.append(acc)
            error.append(abs(self.df_interface.state_data[idx][0] - (data_point[0])))
            error.append(abs(self.df_interface.state_data[idx][3] - (data_point[1])))
            pos_error.append(error)

        self.imm_data["position_measurement"]     = position_measurement
        # self.imm_data["velocity_measurement"]     = velocity_measurement
        # self.imm_data["acceleration_measurement"] = acceleration_measurement

        # state_data
        state_data = self.df_interface.state_data
        position_state, velocity_state, acceleration_state = [], [], []
        velocity_magnitude, velocity_heading, acceleration_magnitude, acceleration_heading = [], [], [], []

        for data_point in state_data:
            pos, vel, acc = [], [], []
            pos.append(data_point[0])
            vel.append(data_point[1])
            acc.append(data_point[2])
            pos.append(data_point[3])
            vel.append(data_point[4])
            acc.append(data_point[5])
            position_state.append(pos)
            velocity_state.append(vel)
            acceleration_state.append(acc)
            velocity_magnitude.append(np.sqrt((data_point[1]**2 + data_point[4]**2)))
            velocity_heading.append(np.arctan((data_point[4], data_point[1])) * 180/np.pi)
            acceleration_magnitude.append(np.sqrt((data_point[2]**2 + data_point[5]**2)))
            acceleration_heading.append(np.arctan((data_point[5], data_point[2])) * 180/np.pi)

        self.imm_data["position_state"]     = position_state
        self.imm_data["velocity_state"]     = velocity_state
        self.imm_data["velocity_magnitude"]     = velocity_magnitude
        self.imm_data["velocity_heading"]     = velocity_heading
        self.imm_data["acceleration_state"] = acceleration_state
        self.imm_data["acceleration_magnitude"] = acceleration_magnitude
        self.imm_data["acceleration_heading"] = acceleration_heading

        # # state_errors
        # state_errors = self.df_interface.state_errors
        # position_errors, velocity_errors, acceleration_errors = [], [], []
        #
        # for data_point in state_errors:
        #     pos, vel, acc = [], [], []
        #     pos.append(data_point[0])
        #     vel.append(data_point[1])
        #     acc.append(data_point[2])
        #     pos.append(data_point[3])
        #     vel.append(data_point[4])
        #     acc.append(data_point[5])
        #     position_errors.append(pos)
        #     velocity_errors.append(vel)
        #     acceleration_errors.append(acc)

        self.imm_data["position_error"]     = pos_error
        # self.imm_data["velocity_error"]     = velocity_errors
        # self.imm_data["acceleration_error"] = acceleration_errors

        # mode_probabilities
        self.imm_data["mode_probabilities"] = self.df_interface.mode_probabilities

    ##############################################################################

    def plot_test_data(self):
        print("Start Plotting test set data")
        x_pos = [pos[0] for pos in self.test_data["position"]]
        y_pos = [pos[1] for pos in self.test_data["position"]]
        x_vel = [vel[0] for vel in self.test_data["velocity"]]
        y_vel = [vel[1] for vel in self.test_data["velocity"]]
        x_acc = [acc[0] for acc in self.test_data["acceleration"]]
        y_acc = [acc[1] for acc in self.test_data["acceleration"]]

        fig, axs = plt.subplots(2, 2)

        axs[0, 0].plot(x_pos, y_pos)
        axs[0, 0].set_title('test_data_position')
        axs[0, 0].set_xlabel("x")
        axs[0, 0].set_ylabel("y")

        axs[1, 0].plot(x_vel, y_vel)
        axs[1, 0].set_title('test_data_velocity')
        axs[1, 0].set_xlabel("x")
        axs[1, 0].set_ylabel("y")

        axs[1, 1].plot(x_acc, y_acc)
        axs[1, 1].set_title('test_data_acceleration')
        axs[1, 1].set_xlabel("x")
        axs[1, 1].set_ylabel("y")
        fig.tight_layout()
        plt.savefig(self.plot_name_test_data, dpi=100)
        plt.show()

        print("Finished Plotting test set data")

    ##############################################################################

    def plot_imm_data(self):
        print("Start Plotting IMM data")
        # x_pos_measurement = [pos[0] for pos in self.imm_data["position_measurement"]]
        # y_pos_measurement = [pos[1] for pos in self.imm_data["position_measurement"]]
        # x_vel_measurement = [vel[0] for vel in self.imm_data["velocity_measurement"]]
        # y_vel_measurement = [vel[1] for vel in self.imm_data["velocity_measurement"]]
        # x_acc_measurement = [acc[0] for acc in self.imm_data["acceleration_measurement"]]
        # y_acc_measurement = [acc[1] for acc in self.imm_data["acceleration_measurement"]]
        x_pos_state       = [pos[0] for pos in self.imm_data["position_state"]]
        y_pos_state       = [pos[1] for pos in self.imm_data["position_state"]]
        x_vel_state       = [vel[0] for vel in self.imm_data["velocity_state"]]
        y_vel_state       = [vel[1] for vel in self.imm_data["velocity_state"]]
        x_acc_state       = [acc[0] for acc in self.imm_data["acceleration_state"]]
        y_acc_state       = [acc[1] for acc in self.imm_data["acceleration_state"]]
        vel_mag       = [vel for vel in self.imm_data["velocity_magnitude"]]
        vel_head       = [vel for vel in self.imm_data["velocity_heading"]]
        acc_mag       = [acc for acc in self.imm_data["acceleration_magnitude"]]
        acc_head       = [acc for acc in self.imm_data["acceleration_heading"]]
        x_pos_error       = [pos[0] for pos in self.imm_data["position_error"]]
        y_pos_error       = [pos[1] for pos in self.imm_data["position_error"]]
        # x_vel_error       = [vel[0] for vel in self.imm_data["velocity_error"]]
        # y_vel_error       = [vel[1] for vel in self.imm_data["velocity_error"]]
        # x_acc_error       = [acc[0] for acc in self.imm_data["acceleration_error"]]
        # y_acc_error       = [acc[1] for acc in self.imm_data["acceleration_error"]]
        mode_prob_1       = [prob[0] for prob in self.imm_data["mode_probabilities"]]
        mode_prob_2       = [prob[1] for prob in self.imm_data["mode_probabilities"]]
        try:
            mode_prob_3       = [prob[2] for prob in self.imm_data["mode_probabilities"]]
        except:
            pass

        ariport_map = plt.imread("D:\\programming\\masterarbeit\\src\\testbench\\EDDH_HAM_Layout.png")
        fig, axs = plt.subplots(1)
        # axs.imshow(ariport_map, extent=[-1730, 1453, -1450, 2930])
        axs.imshow(ariport_map, extent=[-2176, 1644, -1800, 3466])
        #axs.plot(x_pos_measurement, y_pos_measurement, ':', label="Measurement")
        axs.plot(x_pos_state, y_pos_state, label="Dfuse3")
        axs.set_title('IMM Output')
        axs.set_xlabel("x")
        axs.set_ylabel("y")
        axs.legend()

        # axs[1].plot(x_vel_measurement, y_vel_measurement)
        # axs[1].set_title('measurement_velocity')
        # axs[1].set_xlabel("x")
        # axs[1].set_ylabel("y")
        #
        # axs[2].plot(x_acc_measurement, y_acc_measurement)
        # axs[2].set_title('measurement_acceleration')
        # axs[2].set_xlabel("x")
        # axs[2].set_ylabel("y")

        fig.tight_layout()
        plt.savefig(self.plot_name_imm_data + "_output", dpi=500)
        #plt.show()

        fig, axs = plt.subplots(4)
        axs[0].plot(x_vel_state)
        axs[0].set_title('state_vel_x')
        axs[0].set_xlabel("time")
        axs[0].set_ylabel("vel_x")

        axs[1].plot(y_vel_state)
        axs[1].set_title('state_vel_y')
        axs[1].set_xlabel("time")
        axs[1].set_ylabel("vel_y")

        axs[2].plot(vel_mag)
        axs[2].set_title('vel_mag')
        axs[2].set_xlabel("time")
        axs[2].set_ylabel("vel_mag")

        axs[3].plot(vel_head)
        axs[3].set_title('vel_head')
        axs[3].set_xlabel("time")
        axs[3].set_ylabel("vel_head")

        plt.savefig(self.plot_name_imm_data + "_vel_state", dpi=100)
        # plt.show()

        fig, axs = plt.subplots(4)
        axs[0].plot(x_acc_state)
        axs[0].set_title('state_acc_x')
        axs[0].set_xlabel("time")
        axs[0].set_ylabel("acc_x")

        axs[1].plot(y_acc_state)
        axs[1].set_title('state_acc_y')
        axs[1].set_xlabel("time")
        axs[1].set_ylabel("acc_y")

        axs[2].plot(acc_mag)
        axs[2].set_title('acc_mag')
        axs[2].set_xlabel("time")
        axs[2].set_ylabel("acc_mag")

        axs[3].plot(acc_head)
        axs[3].set_title('acc_head')
        axs[3].set_xlabel("time")
        axs[3].set_ylabel("acc_head")

        plt.savefig(self.plot_name_imm_data + "_acc_state", dpi=100)
        # plt.show()

        fig, axs = plt.subplots(1)
        axs.plot(mode_prob_1, label="Mode1")
        axs.plot(mode_prob_2, label="Mode2")
        try:
            axs.plot(mode_prob_3, label="Mode3")
        except:
            pass
        axs.set_xticklabels([])
        axs.set_xticks([])
        axs.set_xlabel("Time")
        axs.set_ylabel("Mode Probability")
        axs.set_title('mode_probabilities')
        axs.set_ylim([0, 1])
        axs.legend()

        fig.tight_layout()
        plt.savefig(self.plot_name_imm_data + "_mode", dpi=100)
        # plt.show()

        fig, axs = plt.subplots(1)
        # axs[0].plot(self.test_data["time"][:-1], x_pos_error, label="Position-x")
        # axs[0].plot(self.test_data["time"][:-1], y_pos_error, label="Position-y")
        # axs[0].set_xticklabels([])
        # axs[0].set_xticks([])
        # axs[0].set_xlabel("Time")
        # axs[0].set_ylabel("Error")
        # axs[0].set_title('Position errors')
        # axs[0].legend()
        # axs[1].plot(self.test_data["time"][:-1], x_vel_error, label="Velocity-x")
        # axs[1].plot(self.test_data["time"][:-1], y_vel_error, label="Velocity-y")
        # axs[1].set_xticklabels([])
        # axs[1].set_xticks([])
        # axs[1].set_xlabel("Time")
        # axs[1].set_ylabel("Error")
        # axs[1].set_title('Velocity errors')
        # axs[1].legend()
        # axs[2].plot(self.test_data["time"][:-1], x_acc_error, label="Acceleration-x")
        # axs[2].plot(self.test_data["time"][:-1], y_acc_error, label="Acceleration-y")
        # axs[2].set_xticklabels([])
        # axs[2].set_xticks([])
        # axs[2].set_xlabel("Time")
        # axs[2].set_ylabel("Error")
        # axs[2].set_title('Acceleration errors')
        # axs[2].legend()
        #
        axs.plot(x_pos_error, label="Position-x")
        axs.plot(y_pos_error, label="Position-y")
        axs.set_xticklabels([])
        axs.set_xticks([])
        axs.set_xlabel("Time")
        axs.set_ylabel("Plot-Track-Diff")
        axs.set_title('Plot-Track-Diff')
        axs.legend()

        plt.savefig(self.plot_name_imm_data + "_Plot-Track-Diff", dpi=100)
        # plt.show()

        print("Finished Plotting IMM data")

    ##############################################################################

    def plot_combined(self):
        print("Start Plotting test set and IMM data")
        x_pos_test_data = [pos[0] for pos in self.test_data["position"]]
        y_pos_test_data = [pos[1] for pos in self.test_data["position"]]
        # x_vel_test_data = [vel[0] for vel in self.test_data["velocity"]]
        # y_vel_test_data = [vel[1] for vel in self.test_data["velocity"]]
        # x_acc_test_data = [acc[0] for acc in self.test_data["acceleration"]]
        # y_acc_test_data = [acc[1] for acc in self.test_data["acceleration"]]
        x_pos_imm_state = [pos[0] for pos in self.imm_data["position_state"]]
        y_pos_imm_state = [pos[1] for pos in self.imm_data["position_state"]]
        # x_vel_imm_state = [vel[0] for vel in self.imm_data["velocity_state"]]
        # y_vel_imm_state = [vel[1] for vel in self.imm_data["velocity_state"]]
        # x_acc_imm_state = [acc[0] for acc in self.imm_data["acceleration_state"]]
        # y_acc_imm_state = [acc[1] for acc in self.imm_data["acceleration_state"]]

        fig, axs = plt.subplots(2, 1)

        axs[0].plot(x_pos_test_data, y_pos_test_data, label="Position test data")
        axs[0].plot(x_pos_imm_state, y_pos_imm_state, label="Position imm state")
        axs[0].set_xlabel("x")
        axs[0].set_ylabel("y")
        axs[0].set_title('Position')
        axs[0].legend()

        # axs[1].plot(x_vel_test_data, y_vel_test_data, label="Velocity test data")
        # axs[1].plot(x_vel_imm_state, y_vel_imm_state, label="Velocity imm state")
        # axs[1].set_xlabel("x")
        # axs[1].set_ylabel("y")
        # axs[1].set_title('Velocity')
        # axs[1].legend()
        #
        # axs[2].plot(x_acc_test_data, y_acc_test_data, label="Acceleration test data")
        # axs[2].plot(x_acc_imm_state, y_acc_imm_state, label="Acceleration imm state")
        # axs[2].set_xlabel("x")
        # axs[2].set_ylabel("y")
        # axs[2].set_title('Acceleration')
        # axs[2].legend()

        fig.tight_layout()
        plt.savefig(self.plot_name_combined_data, dpi=2000)
        plt.show()

        print("Finished Plotting test set and IMM data")

# EOF
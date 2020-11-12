import csv
import os.path as p

import numpy as np

from src.utils.Decorators import *
from src.utils.ConfigParser import ParserLib

# Global Variables
EmptyArray = np.array([])
EmptyList = []

class DataFilteInterface(object):
    @typecheck(str)
    def __init__(self, file_name):
        self.__file_name          = file_name
        self.__measurement_data   = []
        self.__state_data         = []
        self.__mode_probabilities = []
        self.__state_errors       = []
        self.K0 = []
        self.K1 = []
        self.z0 = []
        self.R0 = []
        self.R1 = []
        self.Q0 = []
        self.Q1 = []
        self.x_prior0 = []
        self.x_prior1 = []
        self.x_post0 = []
        self.x_post1 = []
        self.S0 = []
        self.S1 = []

    ##############################################################################

    # Accessors to properties
    @property
    def file_name(self):
        return self.__file_name

    @property
    def measurement_data(self):
        return self.__measurement_data

    @measurement_data.setter
    @typecheck(np.ndarray)
    def measurement_data(self, data):
        self.__measurement_data.append(data)

    @property
    def state_data(self):
        return self.__state_data

    @state_data.setter
    @typecheck(np.ndarray)
    def state_data(self, data):
        self.__state_data.append(data)

    @property
    def mode_probabilities(self):
        return self.__mode_probabilities

    @mode_probabilities.setter
    @typecheck(np.ndarray)
    def mode_probabilities(self, data):
        self.__mode_probabilities.append(data)

    @property
    def state_errors(self):
        return self.__state_errors

    @state_errors.setter
    @typecheck(np.ndarray)
    def state_errors(self, data):
        self.__state_errors.append(data)

    ##############################################################################

    def read(self):
        """
        Reads data from a given file into internal variables
        """
        self.__measurement_data   = []
        self.__state_data         = []
        self.__mode_probabilities = []
        self.__state_errors       = []
        self.K0 = []
        self.K1 = []
        self.z0 = []
        self.R0 = []
        self.R1 = []
        self.Q0 = []
        self.Q1 = []
        self.x_prior0 = []
        self.x_prior1 = []
        self.x_post0 = []
        self.x_post1 = []
        self.S0 = []
        self.S1 = []

        with open(self.__file_name, mode='r') as csv_file:
            csv_reader = csv.DictReader(csv_file, delimiter=";")
            for row in csv_reader:
                self.state_data         = ParserLib.read_matrix(row["state_data"])
                self.mode_probabilities = ParserLib.read_matrix(row["mode_probabilities"])
                self.K0.append(ParserLib.read_matrix(row["K0"]))
                self.K1.append(ParserLib.read_matrix(row["K1"]))
                self.z0.append(ParserLib.read_matrix(row["z"]))
                self.R0.append(ParserLib.read_matrix(row["R0"]))
                self.R1.append(ParserLib.read_matrix(row["R1"]))
                self.Q0.append(ParserLib.read_matrix(row["Q0"]))
                self.Q1.append(ParserLib.read_matrix(row["Q1"]))
                self.x_prior0.append(ParserLib.read_matrix(row["xprior0"]))
                self.x_prior1.append(ParserLib.read_matrix(row["xprior1"]))
                self.x_post0.append(ParserLib.read_matrix(row["xpost0"]))
                self.x_post1.append(ParserLib.read_matrix(row["xpost1"]))
                self.S0.append(ParserLib.read_matrix(row["S0"]))
                self.S1.append(ParserLib.read_matrix(row["S1"]))

    ##############################################################################

    def write(self):
        """
        Writes the data saved in this object to the given file
        """
        with open(self.__file_name, mode='w', newline="") as csv_file:
            fieldnames = ["state_data",
                          "mode_probabilities",
                          "xprior0",
                          "xprior1",
                          "xpost0",
                          "xpost1",
                          "z",
                          "K0",
                          "K1",
                          "R0",
                          "R1",
                          "Q0",
                          "Q1",
                          "S0",
                          "S1",
                          ]
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames, delimiter=";")
            writer.writeheader()

            for line, _ in enumerate(self.state_data):
                state_data         = ParserLib.write_list(self.state_data[line])
                mode_probabilities = ParserLib.write_list(self.mode_probabilities[line])
                xprior0 = ParserLib.write_list(self.x_prior0[line])
                xprior1 = ParserLib.write_list(self.x_prior1[line])
                xpost0 = ParserLib.write_list(self.x_post0[line])
                xpost1 = ParserLib.write_list(self.x_post1[line])
                z = ParserLib.write_list(self.z0[line])
                K0 = ParserLib.write_list(self.K0[line])
                K1 = ParserLib.write_list(self.K1[line])
                R0 = ParserLib.write_list(self.R0[line])
                R1 = ParserLib.write_list(self.R1[line])
                Q0 = ParserLib.write_list(self.Q0[line])
                Q1 = ParserLib.write_list(self.Q1[line])
                S0 = ParserLib.write_list(self.S0[line])
                S1 = ParserLib.write_list(self.S1[line])

                writer.writerow({'state_data': state_data,
                                 'mode_probabilities': mode_probabilities,
                                 'xprior0': xprior0,
                                 'xprior1': xprior1,
                                 'xpost0': xpost0,
                                 'xpost1': xpost1,
                                 'z': z,
                                 'K0': K0,
                                 'K1': K1,
                                 'R0': R0,
                                 'R1': R1,
                                 'Q0': Q0,
                                 'Q1': Q1,
                                 'S0': S0,
                                 'S1': S1
                                 })

    ##############################################################################

    @staticmethod
    def write_test_data(test_data, test_file):
        """
        Writes test data, given as a dictionary, into a csv file to be saved.
        :param test_data: dict - contains the time, position, velocity and acceleration of the test data
        :param test_file: str - path to the test file which shall be written
        """
        time = test_data["time"]          # relative time of series
        pos  = test_data["position"]      # vector of positions
        vel  = test_data["velocity"]      # vector of velocities
        acc  = test_data["acceleration"]  # vector of accelerations

        with open(p.abspath(test_file), mode='w', newline="") as csv_file:
            fieldnames = ["time", "position", "velocity", "acceleration"]
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames, delimiter=";")
            writer.writeheader()

            for line, _ in enumerate(time):
                time_data = time[line]
                pos_data  = ParserLib.write_list(pos[line])
                vel_data  = ParserLib.write_list(vel[line])
                acc_data  = ParserLib.write_list(acc[line])

                writer.writerow({"time": time_data, 'position': pos_data,
                                 'velocity': vel_data, 'acceleration': acc_data})


    ##############################################################################

    @staticmethod
    def read_test_data(test_file):
        """
        Reads test data from a given csv file and saves the content into a dictionary
        :param test_file: str - path to the test file which shall be read
        :return: test_data - dict: contains the read data
        """
        time = []
        pos  = []
        vel  = []
        acc  = []
        with open(p.abspath(test_file), mode='r') as csv_file:
            csv_reader = csv.DictReader(csv_file, delimiter=";")
            for row in csv_reader:
                time.append(row["time"])
                pos.append(ParserLib.read_matrix(row["position"]))
                vel.append(ParserLib.read_matrix(row["velocity"]))
                acc.append(ParserLib.read_matrix(row["acceleration"]))

        test_data = dict()
        test_data["time"]         = time
        test_data["position"]     = pos
        test_data["velocity"]     = vel
        test_data["acceleration"] = acc

        return test_data

    ##############################################################################

    def clear(self):
        """
        Clears all properties except file_name
        """
        for attr, value in self.__dict__.items():
            if attr == "_DataFilteInterface__file_name":
                continue
            setattr(self, attr, [])
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

        with open(self.__file_name, mode='r') as csv_file:
            csv_reader = csv.DictReader(csv_file, delimiter=";")
            for row in csv_reader:
                self.measurement_data   = ParserLib.read_matrix(row["measurement_data"])
                self.state_data         = ParserLib.read_matrix(row["state_data"])
                self.mode_probabilities = ParserLib.read_matrix(row["mode_probabilities"])
                self.state_errors       = ParserLib.read_matrix(row["state_errors"])

    ##############################################################################

    def write(self):
        """
        Writes the data saved in this object to the given file
        """
        with open(self.__file_name, mode='w') as csv_file:
            fieldnames = ["measurement_data", "state_data", "mode_probabilities", "state_errors"]
            writer = csv.DictWriter(csv_file, fieldnames=fieldnames, delimiter=";")
            writer.writeheader()

            for line, _ in enumerate(self.measurement_data):
                measurement_data   = ParserLib.write_list(self.measurement_data[line])
                state_data         = ParserLib.write_list(self.state_data[line])
                mode_probabilities = ParserLib.write_list(self.mode_probabilities[line])
                state_errors       = ParserLib.write_list(self.state_errors[line])

                writer.writerow({"measurement_data": measurement_data, 'state_data': state_data,
                                 'mode_probabilities': mode_probabilities, 'state_errors': state_errors})

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
import csv

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

    def clear(self):
        """
        Clears all properties except file_name
        """
        for attr, value in self.__dict__.items():
            if attr == "_DataFilteInterface__file_name":
                continue
            setattr(self, attr, [])
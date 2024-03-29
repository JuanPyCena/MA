import os
import configparser
import re

import numpy as np

from src.utils.Decorators import *

##############################################################################
# Global variables
EmptyArray = np.array([])
CODEVARIABLES = "code_variables"
REQUIREDPARAMETERS = "required_parameters"

# IMM
FILTERKEY = "filters"
STATEKEY = "state"
MARKOVMATRIXKEY = "markov_transition_matrix"
MODEPROBABILITIESKEY = "mode_probabilities"
EXPANSIONMATRIXKEY = "expansion_matrix"
EXPANSIONMATRIXCOVARIANCEKEY = "expansion_matrix_covariance"
EXPANSIONMATRIXSKEY = "expansion_matrix_s"
SHRINKINGMATRIXKEY = "shrinking_matrix"

# Filter keys
KALMANFILTERKEY = "^(KF+)(\d)*$"  # Regex so multiple KF can be used with numbering
EXTENDEDKALMANFILTERKEY = "^(EKF+)(\d)*$"  # Regex so multiple EKF can be used with numbering
KNOWNFILTERKEYS = [KALMANFILTERKEY, EXTENDEDKALMANFILTERKEY]

# KF/EKF filter
TRANSITIONMATRIX = "transition_matrix"
JACOBIMATRIX = "jacobi_matrix"
INPUTCONTROLMATRIX = "input_control_matrix"
PROCESSNOISEMATRIX = "process_noise_matrix"
COVARIANCEMATRIX = "covariance_matrix"
MEASUREMENTCONTROLMATRIX = "measurement_control_matrix"
MEASUREMENTUNCERTAINTYMATRIX = "measurement_uncertainty_matrix"


##############################################################################

class ParserLib(object):
    @staticmethod
    def read_list(cfg_list):
        """
        The function reads a list which is given as a raw string, checks for the correct syntax and saves it as a
        python list
        :param cfg_list: str - raw string which holds list information, may be nested
        :return: read_list: list - list parsed of cfg_list, bracket syntax is checked. List may be nested
        """
        # check for errors
        brackets = {"opening": ["["], "closing": ["]"]}
        ParserLib._check_for_errors(cfg_list, brackets=brackets)

        # Create a nested list of lists
        read_list = []

        # go through the list symbol by symbol and save each symbol separatly. This allows the usage of nested lists
        intermediate_list = []
        for symbol in cfg_list:
            if symbol in brackets["opening"]:
                intermediate_list = []
            elif symbol in brackets["closing"]:
                if intermediate_list != []:
                    read_list.append("".join(intermediate_list).replace(" ", "").split(","))
                intermediate_list = []
            elif symbol not in brackets["opening"] and symbol not in brackets["closing"]:
                intermediate_list.append(symbol)

        # If config does not give a nested list return the only elemet since we dont want nested lists in this case
        if len(read_list) == 1:
            return read_list[0]

        return read_list

    @staticmethod
    def write_list(np_list):
        """
        The function writes a list in form of a raw string.
        :param np_list: list/np.array - python list or np.array which shall be made into a raw string list
        :return: str - raw string list
        """
        list_str = str(np_list.astype(float))
        # Remove brackets, leading and trailing whitespaces
        list_str = list_str.replace("[ ", "[")
        list_str = list_str[1:-1].strip()
        # Remove doubled whitespaces
        list_str = " ".join(list_str.split())
        # Make whitespace into whitspace plus comma to have a readable list
        list_str = list_str.replace(" ", ", ")
        list_str = list_str.replace("-0.,", "0.,")
        list_str = list_str.replace("0.,", "0,")
        # Add brackets again
        list_str = "[" + list_str + "]"
        list_str = list_str.replace("[,", "[ ")
        list_str = list_str.replace(", ]", "]")
        return list_str

    ##############################################################################

    @staticmethod
    def read_matrix(cfg_matrix, **kwargs):
        """
        The function reads a matrix which is given as a raw string, checks for the correct syntax and saves it as a
        numpy array. This matrix can hold various variables which can be given.
        :param cfg_matrix: str - raw string which holds matrix information, may hold special variables which can
                                 be replaced, or code variables known to the code which stay within the parsed matrix
        :param kwargs: dict - can hold additional information about the "known_variables" which should be replaced
                              within the matrix, or "code_variables" which should not be replaced within the matrix
        :return: np.ndarray - matrix which holds float values read from the raw input string. May hold strings for
                              codes variables
        """
        read_matrix = np.array(ParserLib.read_list(cfg_matrix))
        # In case all values are numeric
        try:
            return read_matrix.astype(np.float)
        # In case we use variables within the matrix replace them with the respective values
        except:
            temp_matrix = np.zeros(read_matrix.shape, dtype=object)
            known_variables = {}
            code_variables = []
            if "known_variables" in kwargs.keys():
                known_variables.update(kwargs["known_variables"])

            if "code_variables" in kwargs.keys():
                code_variables += kwargs["code_variables"]

            if not known_variables:
                raise SyntaxError(
                    "Error while loading configured matrix '{}' !! Non-numeric value found which is no known variable".format(
                        cfg_matrix))

            for row_col, elem in np.ndenumerate(read_matrix):
                try:
                    elem = float(elem)
                except:
                    if elem not in known_variables and elem not in code_variables:
                        SyntaxError(
                            "Error while loading configured matrix '{}' at position '{}' !! Non-numeric value '{}' found which is no known variable".format(
                                cfg_matrix, row_col, elem))
                    if elem not in code_variables:
                        elem = known_variables[elem]

                temp_matrix[row_col] = elem

            read_matrix = temp_matrix

            return read_matrix

    ##############################################################################

    @staticmethod
    def read_known_variables(section, **kwargs):
        """
        The function reads all variables of a given section and save all except for a given list.
        :param section: configparser.SectionProxy - holds the section of the config which should be read
        :param kwargs: dict - holds the required parameters, those are not saved as known variables
        :return: known_variables: dict - holds the known variables for the given section
        """
        known_variables = {}
        required_params = []
        if "required_params" in kwargs.keys():
            required_params += kwargs["required_params"]
        for variable in section:
            if variable in required_params:
                continue
            known_variables[variable] = float(section[variable])

        return known_variables

    ##############################################################################

    @staticmethod
    def calculate_time_depended_matrix(matrix, time, time_variable):
        """
        This function replaces the time depended variable within a given matrix and replaces it with the actual value.
        The timedependend variable may be part of an pytonic mathematical expression (e.g.: dt**4/4), this expression
        is evaulated and the result is entered within the matrix
        :param matrix: np.ndarray - holds data containing timedependend variables which should be replaced with numerical value
        :param time: float - time which should be inserted into the matrix
        :param time_variable: str - key for the time dependend variable, gets replaced by the actual time value
        :return: matrix_replaced: np.ndarray - holds the matrix containing only float values
        """
        matrix_replaced = np.zeros(matrix.shape)
        for row_col, elem in np.ndenumerate(matrix):
            try:
                elem = float(elem)
            except:
                if time_variable not in elem:
                    RuntimeError("Cannot convert the elem '{}' at '{}', since it is no known time variable".format(elem,
                                                                                                                   row_col))
                replaced = elem.replace(time_variable, str(time))
                elem = eval(replaced)
            matrix_replaced[row_col] = elem
        return matrix_replaced.astype(float)

    ##############################################################################

    @staticmethod
    def evaluate_functional_matrix(matrix, time, time_variable, other_variables, other_variables_replacements,
                                   functions, function_replacements):
        """
        This function replaces the time depended variable within a given matrix and replaces it with the actual value.
        The timedependend variable may be part of an pytonic mathematical expression (e.g.: dt**4/4), this expression
        is evaulated and the result is entered within the matrix
        :param matrix: np.ndarray - holds data containing timedependend variables which should be replaced with numerical value
        :param time: float - time which should be inserted into the matrix
        :param time_variable: str - key for the time dependend variable, gets replaced by the actual time value
        :return: matrix_replaced: np.ndarray - holds the matrix containing only float values
        TODO
        """

        def contains_word(s, w):
            return (' ' + w + ' ') in (s)

        def contains_fun(s, w):
            return (w + ' (') in (s)

        def add_whitespace(s):
            ret = [" "]
            for idx, letter in enumerate(s):
                if letter in ["+", "-", "*", "/", "(", ")", ","]:
                    ret.append(" " + letter + " ")
                else:
                    ret.append(letter)
            return "".join(ret)

        matrix_replaced = np.zeros(matrix.shape)
        for row_col, elem in np.ndenumerate(matrix):
            try:
                elem = float(elem)
            except:
                elem = elem.replace(time_variable, str(time))
                elem = add_whitespace(elem)
                for idx, fun in enumerate(functions):
                    if contains_fun(elem, fun):
                        elem = elem.replace(fun, str(function_replacements[idx]))

                for idx, variable in enumerate(other_variables):
                    if contains_word(elem, variable):
                        elem = elem.replace(variable, str(other_variables_replacements[idx]))

                for idx, variable in enumerate(other_variables):
                    elem = elem.replace(variable, str(other_variables_replacements[idx]))

                elem = elem.replace(" ", "")
                elem = elem.replace("0/0", "0")
                if "//" in elem:
                    nom, denom = elem.split("//")
                    nom_val = eval(nom)
                    denom_val = eval(denom)
                    elem = str(nom_val) + "/" + str(denom_val)

                elem = elem.replace("0/0", "0")
                elem = eval(elem)
            matrix_replaced[row_col] = elem
        return matrix_replaced.astype(float)

    ##############################################################################

    @staticmethod
    def _check_for_errors(line, **kwargs):
        # check if all brackets are opened and closed correctly
        if "brackets" in kwargs.keys():
            # check for bracket errors
            ParserLib.__check_brackets(line, kwargs["brackets"])
            # check if line ends with bracket
            if not any(line.endswith(closing) for closing in kwargs["brackets"]["closing"]):
                raise SyntaxError(
                    "Error while loading configuration line '{}' !! Line must end with closing bracket!"
                        .format(line))

    ##############################################################################

    @staticmethod
    def __check_brackets(line, brackets):
        # check if all brackets that are opened have been closed with the correct bracket
        stack = []
        opening_brackets = brackets["opening"]
        closing_brackets = brackets["closing"]
        for symbol in line:
            if symbol in opening_brackets:
                stack.append(symbol)
            elif symbol in closing_brackets and stack == []:
                raise SyntaxError(
                    "Error while loading configuration line '{}' !! Closing bracket without opening it!".format(line))
            elif symbol in closing_brackets and stack != []:
                corresponding_opening = opening_brackets[closing_brackets.index(symbol)]
                last_entry = stack.pop()
                if corresponding_opening != last_entry:
                    raise SyntaxError(
                        "Error while loading configuration line '{}' !! Opened bracket is not closed correctly! '{}' was closed by '{}'!"
                        .format(line, last_entry, symbol))

        if stack != []:
            raise SyntaxError(
                "Error while loading configuration line '{}' !! Not all brackets have been closed!".format(line))

    # EOC


##############################################################################

class ConfigurationParser(object):

    @typecheck(str)
    def __init__(self, **kwargs):
        """
        Creates a ConfigurationParser object which read the configuration of the IMM and all of its filters.
        The configuration values are public varibles of this class
        :param kwargs: dict - holds additional information for the cofniguration (e.g.: cfg_path)
        """
        # public variables
        self.state_variables = []  # list which holds of which type the variables are
        self.filters = []  # list which holds what kind of filters are used for the IMM
        self.markov_transition_matrix = np.array([])  # quadratic matrix which defines the transitions between the modes
        self.mode_probabilities = np.array([])  # vector which holds the probability of each mode
        self.filter_configs = {}  # dict which holds the configs of the filters used of by the IMM
        self.expansion_matrix = np.array([])
        self.expansion_matrix_covariance = np.array([])
        self.expansion_matrix_S = np.array([])
        self.shrinking_matrix = np.array([])

        # private variables
        self.__known_variables = {}
        self.__required_parameters = []  # list which holds the required parameters for the IMM
        self.__code_variables = []  # list which holds variables declared in the config which can be handled within the code

        # local variables
        IMM_cfg_params = {
            "filters": FILTERKEY,
            "state": STATEKEY,
            "markov_transition_matrix": MARKOVMATRIXKEY,
            "mode_probabilities": MODEPROBABILITIESKEY,
            "expansion_matrix": EXPANSIONMATRIXKEY,
            "expansion_matrix_covariance": EXPANSIONMATRIXCOVARIANCEKEY,
            "expansion_matrix_s": EXPANSIONMATRIXSKEY,
            "shrinking_matrix": SHRINKINGMATRIXKEY
        }

        # read config
        self.__config = self.__read_cfg(**kwargs)

        # read general config
        self.__read_general_config()

        # read IMM cfg
        self.__read_IMM_config(IMM_cfg_params)
        # read filter configs for IMM
        self.__read_filter_configs()

        # Verify that the loading has worked
        if not self.__loading_successfull():
            raise AssertionError("Loading of the configuration failed!!")

    ##############################################################################

    @typecheck(str)
    def __read_cfg(self, **kwargs):
        cfg_path = kwargs["cfg_path"]

        # Verify file exists
        if not os.path.isfile(os.path.abspath(cfg_path)):
            raise FileNotFoundError("Config file {file_name} does not exist!"
                                    .format(file_name=os.path.abspath(cfg_path)))

        config = configparser.ConfigParser()
        config.read(os.path.abspath(cfg_path))
        return config

    ##############################################################################

    def __read_general_config(self):
        # Verify Section exists
        if "GENERAL" not in self.__config.sections():
            raise KeyError("Section {section} does not exist"
                           .format(section="GENERAL"))

        self.__code_variables = ParserLib.read_list(self.__config["GENERAL"][CODEVARIABLES])
        self.__required_parameters = ParserLib.read_list(self.__config["GENERAL"][REQUIREDPARAMETERS])
        self.__known_variables.update(ParserLib.read_known_variables(self.__config["GENERAL"],
                                                                     required_params=[CODEVARIABLES,
                                                                                      REQUIREDPARAMETERS]))

    ##############################################################################

    @typecheck(dict)
    def __read_IMM_config(self, cfg_params):
        # Verify Section exists
        if "IMM" not in self.__config.sections():
            raise KeyError("Section {section} does not exist"
                           .format(section="IMM"))

        self.__known_variables.update(ParserLib.read_known_variables(self.__config["IMM"],
                                                                     required_params=self.__required_parameters))
        self.filters = ParserLib.read_list(self.__config["IMM"][cfg_params["filters"]])
        self.state_variables = ParserLib.read_list(self.__config["IMM"][cfg_params["state"]])
        self.markov_transition_matrix = ParserLib.read_matrix(
            self.__config["IMM"][cfg_params["markov_transition_matrix"]],
            code_variables=self.__code_variables,
            known_variables=self.__known_variables)
        self.mode_probabilities = ParserLib.read_matrix(self.__config["IMM"][cfg_params["mode_probabilities"]],
                                                        code_variables=self.__code_variables,
                                                        known_variables=self.__known_variables)
        self.expansion_matrix = ParserLib.read_matrix(self.__config["IMM"][cfg_params["expansion_matrix"]],
                                                      code_variables=self.__code_variables,
                                                      known_variables=self.__known_variables)
        self.expansion_matrix_covariance = ParserLib.read_matrix(
            self.__config["IMM"][cfg_params["expansion_matrix_covariance"]],
            code_variables=self.__code_variables,
            known_variables=self.__known_variables)
        self.expansion_matrix_S = ParserLib.read_matrix(self.__config["IMM"][cfg_params["expansion_matrix_s"]],
                                                        code_variables=self.__code_variables,
                                                        known_variables=self.__known_variables)
        self.shrinking_matrix = ParserLib.read_matrix(self.__config["IMM"][cfg_params["shrinking_matrix"]],
                                                      code_variables=self.__code_variables,
                                                      known_variables=self.__known_variables)

    ##############################################################################

    def __read_filter_configs(self):
        for filter in self.filters:
            if not any(re.compile(regex).match(filter) for regex in KNOWNFILTERKEYS):
                raise SyntaxError("Error while loading configuration for requested filter '{}' !! Filter is not known!!"
                                  .format(filter))

            if re.compile(KALMANFILTERKEY).match(filter):
                kf_config = KalmanFilterConfigParser(self.__config, filter,
                                                     code_variables=self.__code_variables,
                                                     known_variables=self.__known_variables)
                self.filter_configs[filter] = kf_config

            if re.compile(EXTENDEDKALMANFILTERKEY).match(filter):
                ekf_config = ExtendedKalmanFilterConfigParser(self.__config, filter,
                                                              code_variables=self.__code_variables,
                                                              known_variables=self.__known_variables)
                self.filter_configs[filter] = ekf_config

    ##############################################################################

    def __loading_successfull(self):
        if self.filters == []: return False
        if self.state_variables == []: return False
        if self.markov_transition_matrix == EmptyArray: return False
        if self.mode_probabilities == EmptyArray: return False
        if self.filter_configs == {}: return False

        return True

    # EOC


##############################################################################

class KalmanFilterConfigParser(object):

    def __init__(self, config, filter_key, **kwargs):
        """
        Creates a KalmanFilterConfigParser object which reads the section of the configuration of a KalmanFilter
        :param config: ConfigurationParser.ConfigProxy - holds the read configuration
        :param filter_key: str - holds the key used for this filters section
        :param kwargs: dict - can hold additional information like "known_variables" or "code_variables"
        """
        # public variables
        self.transition_matrix = np.array([])  # F, quadratic matrix which defines the transitions of the state
        self.measurement_control_matrix = np.array([])  # H, matrix which holds controls the measurement
        self.input_control_matrix = np.array([])  # B/G, matrix which controls the input
        self.process_noise_matrix = np.array([])  # Q, matrix to control the process noise
        self.covariance_matrix = np.array([])  # P, matrix to hold the initial covariance matrix
        self.measurement_uncertainty_matrix = np.array([])  # R, matrix to hold the measurement uncertainty

        # private variables
        self.__config = config
        self.__known_variables = {}
        self.__code_variables = []
        self.__filter_key = filter_key
        if "known_variables" in kwargs.keys():
            self.__known_variables.update(kwargs["known_variables"])
        if "code_variables" in kwargs.keys():
            self.__code_variables += kwargs["code_variables"]

        # local variables
        KF_cfg_params = {
            "transition_matrix": TRANSITIONMATRIX,
            "input_control_matrix": INPUTCONTROLMATRIX,
            "process_noise_matrix": PROCESSNOISEMATRIX,
            "covariance_matrix": COVARIANCEMATRIX,
            "measurement_control_matrix": MEASUREMENTCONTROLMATRIX,
            "measurement_uncertainty_matrix": MEASUREMENTUNCERTAINTYMATRIX
        }
        self.__known_variables.update(ParserLib.read_known_variables(self.__config[self.__filter_key],
                                                                     required_params=[TRANSITIONMATRIX,
                                                                                      INPUTCONTROLMATRIX,
                                                                                      PROCESSNOISEMATRIX,
                                                                                      COVARIANCEMATRIX,
                                                                                      MEASUREMENTCONTROLMATRIX,
                                                                                      MEASUREMENTUNCERTAINTYMATRIX]))
        self.__read_KF_filter_config(KF_cfg_params)

        # Verify that the loading has worked
        if not self.__loading_successfull():
            raise AssertionError("Loading of the filter '{}' failed!!".format(self.__filter_key))

    ##############################################################################

    def __read_KF_filter_config(self, cfg_params):
        # check if variables are present in config be accessing them
        try:
            _ = self.__config[self.__filter_key][cfg_params["transition_matrix"]]
            _ = self.__config[self.__filter_key][cfg_params["measurement_control_matrix"]]
        except:
            raise SyntaxError(
                "Error while loading configuration for requested filter '{}' !!The following parameters must be configured: '{}'".
                format(self.__filter_key, [TRANSITIONMATRIX, MEASUREMENTCONTROLMATRIX]))

        # Read values which must be present
        self.transition_matrix = ParserLib.read_matrix(
            self.__config[self.__filter_key][cfg_params["transition_matrix"]],
            code_variables=self.__code_variables,
            known_variables=self.__known_variables)
        self.measurement_control_matrix = ParserLib.read_matrix(
            self.__config[self.__filter_key][cfg_params["measurement_control_matrix"]],
            code_variables=self.__code_variables,
            known_variables=self.__known_variables)

        # Optional parameters
        self.input_control_matrix = self.__read_optional_matrix_parameters(INPUTCONTROLMATRIX,
                                                                           cfg_params["input_control_matrix"],
                                                                           self.transition_matrix.shape,
                                                                           optional=np.zeros)
        self.process_noise_matrix = self.__read_optional_matrix_parameters(PROCESSNOISEMATRIX,
                                                                           cfg_params["process_noise_matrix"],
                                                                           self.transition_matrix.shape,
                                                                           optional=np.zeros)
        self.covariance_matrix = self.__read_optional_matrix_parameters(COVARIANCEMATRIX,
                                                                        cfg_params["covariance_matrix"],
                                                                        np.size(self.transition_matrix, 1))
        self.measurement_uncertainty_matrix = self.__read_optional_matrix_parameters(MEASUREMENTUNCERTAINTYMATRIX,
                                                                                     cfg_params[
                                                                                         "measurement_uncertainty_matrix"],
                                                                                     self.transition_matrix.shape,
                                                                                     optional=np.zeros)

    ##############################################################################

    def __read_optional_matrix_parameters(self, praramkey, param, shape, optional=None):
        if praramkey in self.__config[self.__filter_key]:
            return ParserLib.read_matrix(self.__config[self.__filter_key][param],
                                         code_variables=self.__code_variables,
                                         known_variables=self.__known_variables)
        elif optional is None:
            return np.eye(shape)

        return optional(shape)

    ##############################################################################

    def __loading_successfull(self):
        if self.transition_matrix == EmptyArray: return False
        if self.measurement_control_matrix == EmptyArray: return False
        if self.input_control_matrix == EmptyArray: return False
        if self.process_noise_matrix == EmptyArray: return False
        if self.covariance_matrix == EmptyArray: return False
        if self.measurement_uncertainty_matrix == EmptyArray: return False

        return True

    # EOC


##############################################################################

class ExtendedKalmanFilterConfigParser(object):

    def __init__(self, config, filter_key, **kwargs):
        """
        Creates a ExtendedKalmanFilterConfigParser object which reads the section of the configuration of a ExtendedKalmanFilter
        :param config: ConfigurationParser.ConfigProxy - holds the read configuration
        :param filter_key: str - holds the key used for this filters section
        :param kwargs: dict - can hold additional information like "known_variables" or "code_variables"
        """
        # public variables
        self.transition_matrix = np.array([])  # F, quadratic matrix which defines the transitions of the state
        self.jacobi_matrix = np.array([])  # J, jacobi matrix
        self.measurement_control_matrix = np.array([])  # H, matrix which holds controls the measurement
        self.input_control_matrix = np.array([])  # B/G, matrix which controls the input
        self.process_noise_matrix = np.array([])  # Q, matrix to control the process noise
        self.covariance_matrix = np.array([])  # P, matrix to hold the initial covariance matrix
        self.measurement_uncertainty_matrix = np.array([])  # R, matrix to hold the measurement uncertainty

        # private variables
        self.__config = config
        self.__known_variables = {}
        self.__code_variables = []
        self.__filter_key = filter_key
        if "known_variables" in kwargs.keys():
            self.__known_variables.update(kwargs["known_variables"])
        if "code_variables" in kwargs.keys():
            self.__code_variables += kwargs["code_variables"]

        # local variables
        EKF_cfg_params = {
            "transition_matrix": TRANSITIONMATRIX,
            "jacobi_matrix": JACOBIMATRIX,
            "input_control_matrix": INPUTCONTROLMATRIX,
            "process_noise_matrix": PROCESSNOISEMATRIX,
            "covariance_matrix": COVARIANCEMATRIX,
            "measurement_control_matrix": MEASUREMENTCONTROLMATRIX,
            "measurement_uncertainty_matrix": MEASUREMENTUNCERTAINTYMATRIX
        }
        self.__known_variables.update(ParserLib.read_known_variables(self.__config[self.__filter_key],
                                                                     required_params=[TRANSITIONMATRIX, JACOBIMATRIX,
                                                                                      INPUTCONTROLMATRIX,
                                                                                      PROCESSNOISEMATRIX,
                                                                                      COVARIANCEMATRIX,
                                                                                      MEASUREMENTCONTROLMATRIX,
                                                                                      MEASUREMENTUNCERTAINTYMATRIX]))
        self.__read_EKF_filter_config(EKF_cfg_params)

        # Verify that the loading has worked
        if not self.__loading_successfull():
            raise AssertionError("Loading of the filter '{}' failed!!".format(self.__filter_key))

    ##############################################################################

    def __read_EKF_filter_config(self, cfg_params):
        # check if variables are present in config be accessing them
        try:
            _ = self.__config[self.__filter_key][cfg_params["transition_matrix"]]
            _ = self.__config[self.__filter_key][cfg_params["jacobi_matrix"]]
            _ = self.__config[self.__filter_key][cfg_params["measurement_control_matrix"]]
        except:
            raise SyntaxError(
                "Error while loading configuration for requested filter '{}' !!The following parameters must be configured: '{}'".
                    format(self.__filter_key, [TRANSITIONMATRIX, MEASUREMENTCONTROLMATRIX]))

        # Read values which must be present
        self.transition_matrix = ParserLib.read_matrix(
            self.__config[self.__filter_key][cfg_params["transition_matrix"]],
            code_variables=self.__code_variables,
            known_variables=self.__known_variables)
        self.jacobi_matrix = ParserLib.read_matrix(self.__config[self.__filter_key][cfg_params["jacobi_matrix"]],
                                                   code_variables=self.__code_variables,
                                                   known_variables=self.__known_variables)
        self.measurement_control_matrix = ParserLib.read_matrix(
            self.__config[self.__filter_key][cfg_params["measurement_control_matrix"]],
            code_variables=self.__code_variables,
            known_variables=self.__known_variables)

        # Optional parameters
        self.input_control_matrix = self.__read_optional_matrix_parameters(INPUTCONTROLMATRIX,
                                                                           cfg_params["input_control_matrix"],
                                                                           np.size(self.transition_matrix, 1))
        self.process_noise_matrix = self.__read_optional_matrix_parameters(PROCESSNOISEMATRIX,
                                                                           cfg_params["process_noise_matrix"],
                                                                           self.transition_matrix.shape,
                                                                           optional=np.zeros)
        self.covariance_matrix = self.__read_optional_matrix_parameters(COVARIANCEMATRIX,
                                                                        cfg_params["covariance_matrix"],
                                                                        np.size(self.transition_matrix, 1))
        self.measurement_uncertainty_matrix = self.__read_optional_matrix_parameters(MEASUREMENTUNCERTAINTYMATRIX,
                                                                                     cfg_params[
                                                                                         "measurement_uncertainty_matrix"],
                                                                                     self.transition_matrix.shape,
                                                                                     optional=np.zeros)

    ##############################################################################

    def __read_optional_matrix_parameters(self, praramkey, param, shape, optional=None):
        if praramkey in self.__config[self.__filter_key]:
            return ParserLib.read_matrix(self.__config[self.__filter_key][param],
                                         code_variables=self.__code_variables,
                                         known_variables=self.__known_variables)
        elif optional is None:
            return np.eye(shape)

        return optional(shape)

    ##############################################################################

    def __loading_successfull(self):
        if self.transition_matrix == EmptyArray: return False
        if self.jacobi_matrix == EmptyArray: return False
        if self.measurement_control_matrix == EmptyArray: return False
        if self.input_control_matrix == EmptyArray: return False
        if self.process_noise_matrix == EmptyArray: return False
        if self.covariance_matrix == EmptyArray: return False
        if self.measurement_uncertainty_matrix == EmptyArray: return False

        return True

    # EOC
# EOF

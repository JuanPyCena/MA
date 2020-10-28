import abc
import numpy as np
from datetime import datetime

import sqlite3 as sql


class ADBInterface(metaclass=abc.ABCMeta):

    def __init__(self, file: str):
        self._data = dict()

        self._connection = self._create_connection(file)
        self._sensors = [elem[0] for elem in self._connection.execute("SELECT name FROM sensors").fetchall()]
        self._targets = self._getUniqueTargetIDs()

    @property
    def sensors(self) -> list:
        return self._sensors

    @property
    def data(self) -> dict:
        return self._data

    @data.setter
    def data(self, data) -> None:
        self._data.update(data)

    @property
    def connection(self) -> sql.Connection:
        return self._connection

    @property
    def targets(self) -> list:
        return self._targets

    @abc.abstractmethod
    def _generateIMMInput(sel) -> None:
        raise NotImplemented

    def _getUniqueTargetIDs(self):
        return np.unique([elem[0] for elem in self.connection.execute("SELECT target_identification FROM sd_track")
                          if elem[0] is not None]).tolist()

    def _create_connection(self, file) -> sql.Connection:
        try:
            return sql.connect(file)
        except:
            raise ConnectionRefusedError("Cannot connect to \"{}\"".format(file))

    def _getTargetPositions(self, target: str, table: str) -> list:
        pos_cursor = self.connection.execute(
            "SELECT position_x,position_y FROM {} WHERE target_identification=?".format(table),
            (target,))

        return [np.array([x, y]).astype(float) for x, y in pos_cursor.fetchall()]

    def _getTargetCovariance(self, target: str, table: str) -> list:
        cov_cursor = self.connection.execute("SELECT "
                                             "position_x_variance,"
                                             "position_y_variance,"
                                             "position_xy_covariance"
                                             " FROM {} WHERE target_identification=?".format(table),
                                             (target,))

        return [np.array([[cov_x, cov_xy],
                         [cov_xy, cov_y]])
                    .astype(float)
                for cov_x, cov_y, cov_xy in cov_cursor.fetchall()]

    def _getDateTime(self, target: str, table: str) -> list:
        time_cursor = self.connection.execute("SELECT "
                                              "date,"
                                              "time_recording"
                                              " FROM {} WHERE target_identification=?".format(table),
                                              (target,))

        times = []
        for date, time in time_cursor.fetchall():
            d = datetime.strptime(str(date), "%Y%m%d")
            times.append(datetime.fromtimestamp(float(datetime.timestamp(d)) + float(time)))

        return times

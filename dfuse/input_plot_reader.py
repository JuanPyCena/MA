import numpy as np
from datetime import datetime

import sqlite3 as sql


class InputPlotReader(object):

    def __init__(self, file: str, tables: list):
        self._connection = self._create_connection(file)
        self._sensors = [elem[0] for elem in self._connection.execute("SELECT name FROM sensors").fetchall()]
        self._targets = self._getUniqueTargetIDs(tables)

    @property
    def sensors(self) -> list:
        return self._sensors

    @property
    def connection(self) -> sql.Connection:
        return self._connection

    @property
    def targets(self) -> list:
        return self._targets

    def plots(self, table: str) -> list:
        plots = []
        for target_address, target_id, x, y, x_var, y_var, xy_cov, date, time, sensor_id in self.connection.execute("SELECT "
                                       "target_address,"
                                       "target_identification,"
                                       "position_x,"
                                       "position_y,"
                                       "position_x_variance,"
                                       "position_y_variance,"
                                       "position_xy_covariance,"
                                       "date,"
                                       "time_recording,"
                                       "sensor_id "
                                       "FROM {}".format(table)).fetchall():
            if target_id is None:
                continue
            position = np.array([x, -y]).astype(float)
            covariance = np.array([[x_var, -xy_cov], [-xy_cov, y_var]]).astype(float)
            time_of_plot = datetime.fromtimestamp(float(datetime.timestamp(datetime.strptime(str(date), "%Y%m%d"))) + float(time))
            plots.append((target_address, target_id, position, covariance, time_of_plot, sensor_id))

        return plots

    def _getUniqueTargetIDs(self, tables: list):
        target_ids = []
        for table in tables:
            target_ids += np.unique([elem[0] for elem in self.connection.execute(f"SELECT target_identification FROM {table}")
                                     if elem[0] is not None]).tolist()

        return np.unique(target_ids).tolist()

    def _create_connection(self, file) -> sql.Connection:
        try:
            return sql.connect(file)
        except:
            raise ConnectionRefusedError("Cannot connect to \"{}\"".format(file))
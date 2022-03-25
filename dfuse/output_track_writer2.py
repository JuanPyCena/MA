import sqlite3 as sql
from datetime import datetime

from math import sqrt

class OutputTrackWriter(object):

    def __init__(self, file: str):
        self._data = list()
        self._unique_track_ids = dict()
        self._target_adresses = dict()
        self._mode_a = dict()
        self._connection = self._create_connection(file)

    @property
    def target_addresses(self) -> dict:
        return self._target_adresses

    @target_addresses.setter
    def target_addresses(self, addresses: dict) -> None:
        self._target_adresses = addresses
    @property
    def mode_a(self) -> dict:
        return self._mode_a

    @mode_a.setter
    def mode_a(self, mode_a: dict) -> None:
        self._mode_a = mode_a

    @property
    def data(self) -> list:
        return self._data

    @data.setter
    def data(self, data: dict) -> None:
        data_list = []
        for unique_id, target_id in enumerate(data.keys()):
            self._unique_track_ids[target_id] = unique_id
            for idx in range(0, len(data[target_id])):
                pos_x = int(data[target_id][idx][0][0])
                pos_y = -int(data[target_id][idx][0][3])
                vel_x = int(data[target_id][idx][0][1] * 100)  # according to trl
                vel_y = -int(data[target_id][idx][0][4] * 100)  # according to trl
                acc   = round(float(sqrt(data[target_id][idx][0][2]**2 + data[target_id][idx][0][5]**2)), 9)
                var_x = round(data[target_id][idx][1][0][0], 3)
                var_y = round(data[target_id][idx][1][3][3], 3)
                cov_xy = -round(data[target_id][idx][1][3][0], 3)
                date = int(data[target_id][idx][2].strftime("%Y%m%d"))
                time = round(float(datetime.timestamp(data[target_id][idx][2]) - datetime.timestamp(
                    datetime(data[target_id][idx][2].year, data[target_id][idx][2].month,
                             data[target_id][idx][2].day))), 3)
                probs = str([round(prob, 5) for prob in data[target_id][idx][3]]).strip("[]").replace(", ", ";")
                plot_ids = str(data[target_id][idx][4]).strip("[]").replace(",", ";")
                data_point = (target_id, pos_x, pos_y, vel_x, vel_y, acc, var_x, var_y, cov_xy, date, time, plot_ids, probs)
                data_list.append(data_point)

        self._data = sorted(data_list, key=lambda data_point: data_point[9])

    @property
    def connection(self) -> sql.Connection:
        return self._connection

    def writeIMMDataToDatabase(self) -> None:
        self.connection.execute("DROP TABLE IF EXISTS sd_track_dfuse")
        self.connection.commit()
        print("Rename sd_track to sd_track_dfuse")
        self.connection.execute("ALTER TABLE sd_track RENAME TO sd_track_dfuse")
        self.connection.commit()
        print("Create new table sd_track")

        fields = \
            "rec_num INTEGER PRIMARY KEY, " \
            "sensor_id INT, " \
            "position_x INT, " \
            "position_y INT, " \
            "groundspeed_x FLOAT, " \
            "goundspeed_y FLOAT, " \
            "position_x_variance FLOAT, " \
            "position_y_variance FLOAT, " \
            "position_xy_variance FLOAT, " \
            "time_detection FLOAT, " \
            "time_recording FLOAT, " \
            "time_recording_daq_pos FLOAT, " \
            "date INT, " \
            "target_address INT, " \
            "mode_a_code INT, " \
            "target_identification TEXT, " \
            "plot_id_chain TEXT, " \
            "track BOOLEAN, " \
            "smoothed BOOLEAN, " \
            "track_comments TEXT, " \
            "unique_track_num INT, " \
            "latitude FLOAT, " \
            "longitude FLOAT , " \
            "flight_level FLOAT, " \
            "altitude_geometric FLOAT, " \
            "acceleration FLOAT, " \
            "turnrate FLOAT, " \
            "rocd FLOAT, " \
            "orientation FLOAT, " \
            "climbing BOOLEAN, " \
            "descending BOOLEAN, " \
            "accelerating BOOLEAN, " \
            "decelerating BOOLEAN, " \
            "turn_right BOOLEAN, " \
            "turn_left BOOLEAN, " \
            "ground BOOLEAN, " \
            "time_last_mode_a FLOAT, " \
            "time_last_target_address FLOAT, " \
            "time_last_geometric_altitude FLOAT, " \
            "mode_a_code_changed INT, " \
            "psr BOOLEAN, " \
            "ssr BOOLEAN, " \
            "mode_s BOOLEAN, " \
            "uncooperative BOOLEAN, " \
            "plot_id TEXT, " \
            "adsb BOOLEAN, " \
            "mlat BOOLEAN, " \
            "track_id TEXT, " \
            "track_num INT, " \
            "tentative BOOLEAN, " \
            "do_not_show BOOLEAN, " \
            "track_begin BOOLEAN, " \
            "track_recovered BOOLEAN, " \
            "track_end BOOLEAN, " \
            "extrapolated BOOLEAN, " \
            "fused BOOLEAN, " \
            "coasting BOOLEAN, " \
            "vehicle BOOLEAN, " \
            "parking BOOLEAN, " \
            "hijack BOOLEAN, " \
            "radiofailure BOOLEAN, " \
            "simulated BOOLEAN, " \
            "emergency BOOLEAN, " \
            "spi BOOLEAN, " \
            "touchdown_x FLOAT, " \
            "touchdown_y FLOAT, " \
            "touchdown_t FLOAT, " \
            "touchdown_thres_dist FLOAT, " \
            "touchdown_runway_id INT, " \
            "flightplan_id TEXT, " \
            "flightplan_ext_id TEXT, " \
            "flightplan_target_identifaction TEXT, " \
            "range_slant FLOAT, " \
            "azimuth FLOAT, " \
            "target_width FLOAT, " \
            "target_length FLOAT, " \
            "amplitude_max FLOAT, " \
            "chain INT"

        self.connection.execute(f"CREATE TABLE sd_track({fields})")

        print("Start writing to database")
        rec_num = 1
        for target_id, pos_x, pos_y, vel_x, vel_y, acc, var_x, var_y, cov_xy, date, time, plot_ids, probs in self.data:
            unique_track_num = self._unique_track_ids[target_id]
            target_address   = self.target_addresses[target_id]
            mode_a   = self.mode_a[target_id]

            row = f"{rec_num}, 0, {pos_x}, {pos_y}, {vel_x}, " \
                  f"{vel_y}, {var_x}, {var_y}, " \
                  f"{cov_xy}, {time}, {time}, {time}, " \
                  f"{date}, {target_address},  {mode_a}, \"{target_id}\", " \
                  f"\"{plot_ids}\", 1, 1, \"{probs}\", {unique_track_num}, "\
                  f"53.62399885, 9.979973221, 1, 2147483647, {acc}, 0, 0, "\
                  f"1000, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, \"{target_id}\", 0, 0, \"{target_id}\", " \
                  f"{unique_track_num}, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, " \
                  f"\"{target_id}\", \"{target_id}\", \"{target_id}\", 0, 0, 10000, 10000, 0, 0"

            query = f"INSERT INTO sd_track VALUES ({row})"
            self.connection.execute(query)
            rec_num += 1
        self.connection.commit()

        print("Finished writing to database")

    def _create_connection(self, file) -> sql.Connection:
        try:
            return sql.connect(file)
        except:
            raise ConnectionRefusedError("Cannot connect to \"{}\"".format(file))

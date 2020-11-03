from dfuse.dfuse3 import DFuse3
from dfuse.input_plot_reader import InputPlotReader
from datetime import datetime
from datetime import timedelta

from src.testbench.TestbenchPlotter import TestbenchPlotter
from src.utils.DataFileInterface import DataFilteInterface as DFI
from src.utils.logmod import Logger

log = Logger()


def main():
    dfuse = DFuse3()

    input_reader = InputPlotReader("D:\\programming\\masterarbeit\\data\\test_eval_test_run_info.adb")

    mlat_plots = input_reader.plots("sd_mlat")

    extrapolation_time = mlat_plots[0][3]
    input_data = []
    plot_uuid = 0
    i = 0
    while i < len(mlat_plots):
        target_id, position, covariance, time = mlat_plots[i]

        if time - extrapolation_time >= timedelta(seconds=1):
            extrapolation_time = extrapolation_time + timedelta(seconds=1)
            input_data.append((target_id, position, covariance, extrapolation_time, str(plot_uuid), True))
        else:
            input_data.append((target_id, position, covariance, time, str(plot_uuid), False))
            i += 1
            plot_uuid += 1

    plot_chain = {}
    for target in input_reader.targets:
        plot_chain[target] = []

    log.write_to_log("Start Simulation", consoleOutput=True)

    # Go through the whole input data and extrapolate every time 1 second has passed
    for target_id, position, covariance, time, plot_id, extrapolate in input_data:
        if extrapolate:
            dfuse.extrapolate(plot_chain, time)
            # Reset plot chain
            for target in input_reader.targets:
                plot_chain[target] = []
            continue

        # IMM calculation
        dfuse.add_plot_data(target_id, position, covariance, time)

        plot_chain[target_id].append(plot_id)

    log.write_to_log("Stop Simulation", consoleOutput=True)

    imm_output = dfuse.extrapolated_targets

    for target in imm_output.keys():
        imm_data_file = "D:\\programming\\masterarbeit\\\src\\testbench\\imm_data\\imm_data_{}.csv".format(target)
        imm_data_writer = DFI(imm_data_file)

        for state, covariance, mode_probabilities, _ in imm_output[target]:
            imm_data_writer.state_data = state.copy()
            imm_data_writer.mode_probabilities = mode_probabilities.copy()

        imm_data_writer.write()
        print("Data saved in: {}".format(imm_data_writer.file_name))
        plotter = TestbenchPlotter("D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv", imm_data_file)
        plotter.plot_imm_data()


if __name__ == "__main__":
    main()

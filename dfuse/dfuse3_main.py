# from dfuse.dfuse3 import DFuse3
from dfuse.dfuse3_filterpy import DFuse3
# from dfuse.dfuse3_kalman_only import DFuse3
from dfuse.input_plot_reader import InputPlotReader
from dfuse.output_track_writer import OutputTrackWriter

from datetime import datetime
from datetime import timedelta

from src.testbench.TestbenchPlotter import TestbenchPlotter
from src.utils.DataFileInterface import DataFilteInterface as DFI
from src.utils.logmod import Logger

log = Logger()

def getInputPlots(file: str, tables: list) -> list:
    input_reader = InputPlotReader(file, tables)
    plots = []
    for table in tables:
        plots += input_reader.plots(table)

    return input_reader.targets, sorted(plots, key=lambda plot: datetime.timestamp(plot[6]))

def createInputData(sensor_plots: list) -> tuple:
    # Create input data with unique id. Set extrapolation flag to true every 1 second to force extrapolation at 1 sencond intervals
    extrapolation_time = sensor_plots[0][6]
    input_data = []
    plot_uuid = 0
    i = 0
    target_addresses = dict()
    mode_a_dict = dict()
    vehicle_dict = dict()
    while i < len(sensor_plots):
        mode_a, target_address, vehicle, target_id, position, covariance, time, sensor_id = sensor_plots[i]
        if target_id not in target_addresses.keys():
            target_addresses[target_id] = target_address
            mode_a_dict[target_id] = mode_a
            vehicle_dict[target_id] = vehicle

        if time - extrapolation_time >= timedelta(seconds=1):
            extrapolation_time = extrapolation_time + timedelta(seconds=1)
            input_data.append((target_id, position, covariance, extrapolation_time, str(plot_uuid), True, sensor_id))
        else:
            input_data.append((target_id, position, covariance, time, str(plot_uuid), False, sensor_id))
            i += 1
            plot_uuid += 1

    return mode_a_dict, target_addresses, vehicle_dict, input_data

def main():
    dfuse = DFuse3()

    targets, sensor_plots = getInputPlots("D:\\programming\\masterarbeit\\data\\test_eval_test_run_info.adb", ["sd_mlat", "sd_adsb"])
    output_writer = OutputTrackWriter("D:\\programming\\masterarbeit\\data\\test_eval_test_run_info.adb")

    mode_a, target_addresses, vehicle, input_data = createInputData(sensor_plots)

    dfuse.vehicle = vehicle

    # dictionary to save plot chain ids
    plot_chain = {}
    for target in targets:
        plot_chain[target] = []

    log.write_to_log("Start Simulation", consoleOutput=True)

    for target_id, position, covariance, time, plot_id, extrapolate, sensor_id in input_data:

        # Extrapolation every second
        if extrapolate:
            dfuse.extrapolate(plot_chain, time)
            # Reset plot chain
            for target in targets:
                plot_chain[target] = []
            continue

        # IMM calculation
        dfuse.add_plot_data(target_id, position, covariance, time, sensor_id)

        plot_chain[target_id].append(plot_id)

    log.write_to_log("Stop Simulation", consoleOutput=True)

    imm_input     = dfuse.target_plots
    imm_outliers  = dfuse.target_outliers
    imm_output    = dfuse.extrapolated_targets

    output_writer.target_addresses = target_addresses
    output_writer.mode_a = mode_a
    output_writer.data = imm_output
    output_writer.writeIMMDataToDatabase()

    # write data to csv and plot
    for target in imm_output.keys():
        imm_data_file = "D:\\programming\\masterarbeit\\\src\\testbench\\imm_data\\imm_data_{}.csv".format(target)
        imm_data_writer = DFI(imm_data_file)

        for state, _, _, mode_probabilities, _, metadata_f0, metadata_f1 in imm_output[target]:
            state_prior, state_post, K, system_uncertainty, process_noise_function, state_uncertainty, measurement = metadata_f0
            imm_data_writer.x_prior0.append(state_prior.copy())
            imm_data_writer.x_post0.append(state_post.copy())
            imm_data_writer.K0.append(K.copy())
            imm_data_writer.R0.append(system_uncertainty.copy())
            imm_data_writer.Q0.append(process_noise_function.copy())
            imm_data_writer.S0.append(state_uncertainty.copy())
            imm_data_writer.z0.append(measurement.copy())

            state_prior, state_post, K, system_uncertainty, process_noise_function, state_uncertainty, measurement = metadata_f1
            imm_data_writer.x_prior1.append(state_prior.copy())
            imm_data_writer.x_post1.append(state_post.copy())
            imm_data_writer.K1.append(K.copy())
            imm_data_writer.R1.append(system_uncertainty.copy())
            imm_data_writer.Q1.append(process_noise_function.copy())
            imm_data_writer.S1.append(state_uncertainty.copy())

            imm_data_writer.state_data = state.copy()
            imm_data_writer.mode_probabilities = mode_probabilities.copy()

        imm_data_writer.write()
        print("Data saved in: {}".format(imm_data_writer.file_name))
        plotter = TestbenchPlotter(
            "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv",
            imm_data_file)
        plotter.plot_imm_data()

    # # write data to csv and plot
    # for target in imm_outliers.keys():
    #     imm_data_file = "D:\\programming\\masterarbeit\\\src\\testbench\\imm_data\\imm_data_outlier_{}.csv".format(target)
    #     imm_data_writer = DFI(imm_data_file)
    #
    #     for _, plot_position, _ in imm_outliers[target]:
    #         imm_data_writer.plot_outlier = plot_position.copy()
    #
    #     imm_data_writer.write_outliers()
    #     print("Data saved in: {}".format(imm_data_writer.file_name))
    #     plotter = TestbenchPlotter("D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv", imm_data_file, False, True)
    #     plotter.plot_imm_outliers()
    #
    # # write data to csv and plot
    # for target in imm_input.keys():
    #     imm_data_file = "D:\\programming\\masterarbeit\\\src\\testbench\\imm_data\\imm_data_input_{}.csv".format(target)
    #     imm_data_writer = DFI(imm_data_file)
    #
    #     for _, plot_position, _ in imm_input[target]:
    #         imm_data_writer.plot_data = plot_position.copy()
    #
    #     imm_data_writer.write_input()
    #     print("Data saved in: {}".format(imm_data_writer.file_name))
    #     plotter = TestbenchPlotter("D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_acc_then_const.csv", imm_data_file, True)
    #     plotter.plot_imm_input()


    # Save IMM output to database


if __name__ == "__main__":
    main()

# Ideen:
# Weiterer Kalman Filter für den Output des IMM. IMM -> Kalman -> output
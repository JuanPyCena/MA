from src.testbench.TestbenchPlotter import TestbenchPlotter
from src.testbench.TestbenchIMM import TestbenchIMM
from src.utils.DataFileInterface import DataFilteInterface as DataReadWrite

def main():
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion_non_uniform_time.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_linear_uniform_motion_non_uniform_time.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()

    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_turning_motion_non_uniform_time.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_constant_turning_non_motion_non_uniform_time.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()
    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_linear_acceleration_motion_non_uniform_time.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_constant_linear_acceleration_motion_non_uniform_time.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()

    test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion.csv"
    imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_linear_uniform_motion.csv"
    test_data = DataReadWrite.read_test_data(test_data_file)

    imm = TestbenchIMM(test_data, imm_data_file)
    imm.run()
    plotter = TestbenchPlotter(test_data_file, imm_data_file)

    plotter.plot_test_data()
    plotter.plot_imm_data()
    plotter.plot_combined()
    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_turning_motion.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_constant_turning_motion.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()

    test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_linear_acceleration_motion.csv"
    imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_constant_linear_acceleration_motion.csv"
    test_data = DataReadWrite.read_test_data(test_data_file)

    imm = TestbenchIMM(test_data, imm_data_file)
    imm.run()
    plotter = TestbenchPlotter(test_data_file, imm_data_file)

    plotter.plot_test_data()
    plotter.plot_imm_data()
    plotter.plot_combined()
    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_linear_uniform_motion_noise.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_linear_uniform_motion_noise.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()
    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_turning_motion_noise.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_constant_turning_motion_noise.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()
    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_constant_linear_acceleration_motion_noise.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_constant_linear_acceleration_motion_noise.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()
    #
    # test_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\\src\\testbench\\test_data\\test_data_combined_motion.csv"
    # imm_data_file = "D:\\programming\\pycharm\\Masterarbeit\\MA\src\\testbench\\imm_data\\imm_data_combined_motion.csv"
    # test_data = DataReadWrite.read_test_data(test_data_file)
    #
    # imm = TestbenchIMM(test_data, imm_data_file)
    # imm.run()
    # plotter = TestbenchPlotter(test_data_file, imm_data_file)
    #
    # plotter.plot_test_data()
    # plotter.plot_imm_data()
    # plotter.plot_combined()

if __name__ == "__main__":
    main()
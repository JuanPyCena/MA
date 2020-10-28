from src.database.ADBConnector import ADBConnector


def main():
    connector = ADBConnector("D:\\programming\\masterarbeit\\data\\test_eval_test_run_info.adb")
    print(connector.data)
if __name__ == "__main__":
    main()
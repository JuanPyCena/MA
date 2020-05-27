# This template file is copied into every package which does not require a custom build procedure.
# It contains the boilerplate code to delegate building to the build system.
#
# *ATTENTION* If you need to change this file, ensure that the template in the template code package is updated.

import os
import sys
import subprocess
import imp

#------------------------------------------------------------------------------
def checkout_build_system():
    dirname = os.path.dirname(os.path.abspath(__file__))

    # cygwin is used for checkout_build_system.sh because git only exists in cygwin on windows build agents :(
    if os.name == 'nt':
        cygpath_script_command = "cygpath -u \"" + dirname + "\\checkout_build_system.sh\""
        checkout_command = subprocess.check_output([r"C:\cygwin\bin\bash.exe", "--login", "-c", cygpath_script_command])

        cyg_cd = "cd $(cygpath -u \"" + os.getcwd() + "\");"
        cyg_checkout_command = cyg_cd + checkout_command

        command = [r"C:\cygwin\bin\bash.exe", "--login", "-c", cyg_checkout_command]
    else:
        checkout_command = dirname + "/checkout_build_system.sh"
        command = [checkout_command]

    subprocess.check_call(command)

#------------------------------------------------------------------------------
def run_build():
    try:
        module_source = imp.load_source("default_build", "build-system/scripts/default_build.py")
        module_source.run_default_build()
    except IOError:
        print("Error: could not load run build script.")
        raise

#------------------------------------------------------------------------------
def set_avbuild_environment():
    dirname = os.path.dirname(os.path.abspath(__file__))
    try:
        module_source = imp.load_source("build_package_details", dirname+"/build_package_details.py")
        module_source.set_avbuild_environment()
    except IOError:
        print("Info: custom build package details module not found.")

#------------------------------------------------------------------------------
if __name__ == "__main__":
    try:
        checkout_build_system()
        set_avbuild_environment()
        run_build()
    except (subprocess.CalledProcessError, IOError) as e:
        print("Operation failed. Build aborted!\n" + str(e))
        sys.exit(1)
    except Exception as e:
        print("\nUnexpected error: \n" + str(e))
        sys.exit(1)

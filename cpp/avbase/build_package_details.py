
import os

#------------------------------------------------------------------------------
def set_avbuild_environment():
    # work around BS-694 - don't run parallel tests for CentOS6.6
    if os.environ.get("bamboo_job_type", '') == "normal_centos6.6":
        return

    # finding a proper value here is tracked in ATC-3652
    os.environ['avbuild_test_container_count'] = '4'


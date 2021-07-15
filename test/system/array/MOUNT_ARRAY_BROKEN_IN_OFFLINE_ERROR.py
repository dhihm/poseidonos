#!/usr/bin/env python3
import subprocess
import os
import sys
sys.path.append("../lib/")

import json_parser
import pos
import pos_util
import cli
import test_result
import json
import time
import CREATE_ARRAY_NO_SPARE
ARRAYNAME = CREATE_ARRAY_NO_SPARE.ARRAYNAME

def check_result(detail):
    if json_parser.is_online(detail) == False:
        return "pass"
    return "fail"

def set_result(detail):
    out = detail
    code = json_parser.get_response_code(detail)
    result = test_result.expect_false(code)
    if result == "pass":
        out = cli.array_info(ARRAYNAME)
        result = check_result(out)
    
    with open(__file__ + ".result", "w") as result_file:
        result_file.write(result + " (" + str(code) + ")" + "\n" + out)

def execute():
    CREATE_ARRAY_NO_SPARE.execute()
    pos_util.pci_detach(CREATE_ARRAY_NO_SPARE.DATA_DEV_1)
    time.sleep(0.1)
    pos_util.pci_detach(CREATE_ARRAY_NO_SPARE.DATA_DEV_2)
    time.sleep(0.1)
    out = cli.mount_array(CREATE_ARRAY_NO_SPARE.ARRAYNAME)
    return out

if __name__ == "__main__":
    test_result.clear_result(__file__)
    out = execute()
    set_result(out)
    pos.kill_pos()
    pos_util.pci_rescan()
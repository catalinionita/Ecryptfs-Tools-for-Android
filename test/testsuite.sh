#!/bin/bash
#  SMOKE Testsuite
# * Copyright (C) 2013 Intel Corporation, All Rights Reserved
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *      http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
# * Author: Razvan-Costin Ionescu <razvanx.ionescu@intel.com>
# */

res=`adb shell 'cat proc/kallsyms | grep -c ecryptfs'`
res=${res:0:1}
if [ "$res" -eq 0 ]
then
   echo "Ecryptfs module is not present. Please use a kernel with ecrypfs module active."
   exit 1;
else
   echo "Ecryptfs kernel module detected. Proceed with test sequence."
fi

echo 'Starting secure containers test:'
source test_functions.sh
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
setup $STORAGE_PATH
create_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH
remove_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
setup $STORAGE_PATH
create_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH
check_labels $ENCRYPTED_STORAGE_PATH
unlock_storage $STORAGE_PATH $OLD_PASS
lock_storage $STORAGE_PATH
change_passwd $STORAGE_PATH $OLD_PASS qwe
change_passwd $STORAGE_PATH $OLD_PASS $NEW_PASS
unlock_storage $STORAGE_PATH $NEW_PASS
lock_storage $STORAGE_PATH
restore_storage $STORAGE_PATH $NEW_PASS
clean "/data/data/bla" "/data/data/.bla"
setup "/data/data/bla"
create_storage "/data/data/bla" bla "/data/data/.bla"
create_storage "/../../data/data/bla" test1 "/./../data/data/.bla"
remove_storage "/../../data/data/bla" test1 "/./../data/data/.bla"
echo 'Starting native daemon integration tests'
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
setup $STORAGE_PATH
edc_create_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH
edc_remove_storage $STORAGE_PATH
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
setup $STORAGE_PATH
edc_create_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH
edc_unlock_storage $STORAGE_PATH $OLD_PASS
edc_lock_storage $STORAGE_PATH
edc_change_passwd $STORAGE_PATH $OLD_PASS $NEW_PASS
edc_unlock_storage $STORAGE_PATH $NEW_PASS
edc_lock_storage $STORAGE_PATH
edc_recover_storage $STORAGE_PATH $NEW_PASS
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
clean "/data/data/bla" "/data/data/.bla"
setup "/data/data/bla"
edc_create_storage "/data/data/bla" bla "/data/data/.bla"
edc_create_storage "/../../data/data/bla" test1 "/./../data/data/.bla"
edc_remove_storage "/../../data/data/bla" test1 "/./../data/data/.bla"
echo "Running progress test"
./progress_test.sh
echo "Running stress test"
./stress_test.py --max-size 1MB --max-level 3 --max-files 5 --max-links 10


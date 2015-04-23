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

source smoke_with_fun.sh
#Test suite
#Positive testing
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
#############
#Negative testing
clean "/data/data/bla" "/data/data/.bla"
setup "/data/data/bla"
#Password too short
create_storage "/data/data/bla" bla "/data/data/.bla"
create_storage "/../../data/data/bla" test1 "/./../data/data/.bla"
remove_storage "/../../data/data/bla" test1 "/./../data/data/.bla"

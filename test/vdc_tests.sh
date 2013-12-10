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
#Positive testing
echo 'Positive testing with VDC'
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
setup $STORAGE_PATH
vdc_create_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH
vdc_remove_storage $STORAGE_PATH

clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH
setup $STORAGE_PATH
vdc_create_storage $STORAGE_PATH $OLD_PASS $ENCRYPTED_STORAGE_PATH

vdc_unlock_storage $STORAGE_PATH $OLD_PASS
vdc_lock_storage $STORAGE_PATH
vdc_change_passwd $STORAGE_PATH $OLD_PASS $NEW_PASS
vdc_unlock_storage $STORAGE_PATH $NEW_PASS
vdc_lock_storage $STORAGE_PATH
vdc_recover_storage $STORAGE_PATH $NEW_PASS
clean $STORAGE_PATH $ENCRYPTED_STORAGE_PATH

#Negative testing
echo "Negative tests with VDC"
clean "/data/data/bla" "/data/data/.bla"
setup "/data/data/bla"
#Password too short
vdc_create_storage "/data/data/bla" bla "/data/data/.bla"
vdc_create_storage "/../../data/data/bla" test1 "/./../data/data/.bla"
vdc_remove_storage "/../../data/data/bla" test1 "/./../data/data/.bla"

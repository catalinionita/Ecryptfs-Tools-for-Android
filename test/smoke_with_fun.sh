#!/bin/bash

#  Testing library for libefs containing functions for create, lock/unlock, restore, remove and change password for a storage container
#
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

#declaring constants
OLD_PASS='testold'
NEW_PASS='testnew'
STORAGE_PATH='/data/data/new_folder'
ENCRYPTED_STORAGE_PATH='/data/data/.new_folder'

#this is a hardcoded key, based on the hardcoded STORAGE_PATH. if the STORAGE_PATH is changed, you should change also the value of KEY, otherwise the create/remove tests will falsely FAIL
KEY='.keys.b318b6b72bd2dae7b992fe2cbf5c42b45914c3c9597a9b1732af6d25b5df3f6c0ad517bc67ce5d33d8b19c4f4adb910dd894cde8ffd7023eaceffa00bb395ac0'

#declaring vars
extratag=logs_`date +"%s"`

if [[ $# -eq "2" && $1 -eq "-o" ]]
then
   test -d "$2" || mkdir -p "$2"
   logfolder=$2
else
   test -d "$extratag" || mkdir -p "$extratag"
   logfolder=$extratag
fi

#cleaning up
function clean() {
	adb shell umount $1 > /dev/null
	adb shell umount $2 > /dev/null
	adb shell 'rm -rf '$1
	adb shell 'rm -rf '$2
	adb logcat -c
}

#here add a setup function where to populate the STORAGE_PATH with files
function setup(){
	adb shell mkdir $1
	adb shell 'echo aaa > '$1'/a.txt'
	adb shell 'echo bbb > '$1'/b.txt'
	adb shell 'echo ccc > '$1'/c.txt'
}
function create_storage() {
	test1='CREATE a libefs '$1' container'
	adb shell efs-tools storage create $1 $2
	if [[ `expr length $2` -le "3" ]]
		then
			ok0=$(adb logcat -d > $logfolder/log_test1.log && grep -c 'Passwd too short' $logfolder/log_test1.log)
			if [[ $ok0 == "1" ]]
				then
					echo -e "\n$test1 did not succeed - password is too short - PASS"
				else
					echo -e "\n$test1 did not succeed - password is too short - FAIL"
			fi

		else
			#verify logcat
			ok1=$(adb logcat -d > $logfolder/log_test1.log && grep -c 'Secure storage created for '$1 $logfolder/log_test1.log)
			#ENCRYPTED_STORAGE_PATH should contain the encrypted files
			ok2=$(adb shell ls $3 | grep -c ECRYPTFS)
			ok3=$(adb shell ls -la /data/misc/keystore/ | grep -c $KEY)  
			#if [[ $ok1 == "1" && $ok2 == "3" && $ok3 == "1" ]]		#if the hardcoded KEY is used ok3 is needed
			if [[ $ok1 == "1" && $ok2 == "3" ]]
			then
				echo -e "\n$test1 - PASS"
			else
				echo -e "\n$test1 - FAIL"
			fi
	fi
	adb logcat -c
}

function unlock_storage() {
	test2='UNLOCK the libefs '$1' container'
	adb shell efs-tools storage unlock $1 $2
	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_test2.log && grep -c 'Secure storage '$1' unlocked' $logfolder/log_test2.log)
	#$STORAGE_PATH should contain the files in plain text
	ok2=$(adb shell ls $1| grep -c txt)
	if [[ $ok1 == "1" && $ok2 == "3" ]]
	then
		echo -e "\n$test2 - PASS"
	else
		echo -e "\n$test2 - FAIL"
	fi

	adb logcat -c
}

function lock_storage() {
	test3='LOCK the libefs '$1' container'
	adb shell efs-tools storage lock $1
	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_test3.log && grep -c 'Secure storage '$1' locked' $logfolder/log_test3.log)

	#$STORAGE_PATH should be empty
	ok2=$(adb shell ls $1 | grep -c txt)

	if [[ $ok1 == "1" && $ok2 == "0" ]]
	then
		echo -e "\n$test3 - PASS"
	else
		echo -e "\n$test3 - FAIL"
	fi
	adb logcat -c
}

function change_passwd() {
	test4='CHANGE PASSWORD for the libefs '$1' container'
	#cleaning the logcat again
	adb logcat -c
	adb shell efs-tools storage change_passwd $1 $2 $3
      	if [[ `expr length $3` -le "3" ]]
		then
			ok0=$(adb logcat -d >  $logfolder/log_test4.log && grep -c 'New passwd too short' $logfolder/log_test4.log)
			if [[ $ok0 == "1" ]]
				then
					echo -e "\n$test4 did not succeed - password is too short - PASS"
				else
					echo -e "\n$test4 did not succeed - password is too short - FAIL"
			fi

		else
			#verify logcat
			ok1=$(adb logcat -d >  $logfolder/log_test4.log && grep -c 'Change passwd successful for '$1' storage' $logfolder/log_test4.log)
			if [[ $ok1 == "1" ]]
			then
				echo -e "\n$test4 from: $2 to: $3 - PASS"
			else
				echo -e "\n$test4 from: $2 to: $3 - FAIL"
		fi
        fi
	adb logcat -c
}

function restore_storage() {
	test5='RESTORE the libefs '$1' container'
	
	adb shell efs-tools storage restore $1 $2

	#verify logcat
	ok3=$(adb logcat -d >  $logfolder/log_test5.log && grep -c 'Secure storage '$1' removed' $logfolder/log_test5.log)
	ok4=$(adb logcat -d >  $logfolder/log_test5.log && grep -c 'Secure storage '$1' restored' $logfolder/log_test5.log)

	#ENCRYPTED_STORAGE_PATH folder does not exist anymore
	ok5=$(adb shell ls $ENCRYPTED_STORAGE_PATH | grep -c ECRYPTFS)
	#STORAGE_PATH folder contains the files in plaintext
	ok6=$(adb shell ls $1 | grep -c txt)

	if [[ $ok3 == "1" && $ok4 == "1" && $ok5 == "0" && $ok6 == "3" ]]
	then
		echo -e "\n$test5 - PASS"
	else
		echo -e "\n$test5 - FAIL"
	fi
	adb logcat -c
}

function remove_storage() {
	test6='REMOVE the libefs container'
	adb shell efs-tools storage remove $1
	#keys are no more present in /data/misc/keystore
	ok1=$(adb shell ls -la /data/misc/keystore/ | grep -c $KEY)  
	#verify logcat
	ok3=$(adb logcat -d >  $logfolder/log_test6.log && grep -c 'Secure storage '$1' removed' $logfolder/log_test6.log)
	#STORAGE_PATH and ENCRYPTED_STORAGE_PATH folders are now removed/empty
	ok4=$(adb shell ls $3 | grep -c ECRYPTFS)
	ok5=$(adb shell ls $1 | grep -c txt)
	#if [[ $ok1 == "0" && $ok3 == "1" && $ok4 == "0" && $ok5 == "0" ]]   #if the hardcoded KEY is used ok1 is needed
	if [[ $ok3 == "1" && $ok4 == "0" && $ok5 == "0" ]]
	then
		echo -e "\n$test6 - PASS"
	else
		echo -e "\n$test6 - FAIL"
	fi
}

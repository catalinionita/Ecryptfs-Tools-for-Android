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
LOST_FOUND='/data/lost+found/new_folder'

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

function set_selinux_enforce() {
	adb shell "setenforce $1" > /dev/null
}

#cleaning up
function clean() {
	adb shell umount $1 > /dev/null
	adb shell umount $2 > /dev/null
	adb shell 'rm -rf '$1
	adb shell 'rm -rf '$2
	adb shell 'rm -rf '$LOST_FOUND
	adb logcat -c
}


#here add a setup function where to populate the STORAGE_PATH with files
function setup(){
	set_selinux_enforce 0
	adb shell mkdir $1
	adb shell 'echo aaa > '$1'/a.txt'
	adb shell 'echo bbb > '$1'/b.txt'
	adb shell 'echo ccc > '$1'/c.txt'
	adb shell 'chcon u:test_a:test_a:s0 '$1'/a.txt'
	adb shell 'chcon u:test_b:test_b:s0 '$1'/b.txt'
	adb shell 'chcon u:test_c:test_c:s0 '$1'/c.txt'
}
function create_storage() {
	test='Create secure container '$1
	adb shell efs-tools storage create $1 $2
	if [[ `expr length $2` -le "3" ]]
		then
			ok0=$(adb logcat -d > $logfolder/log_create_storage.log && grep -c 'Passwd too short' $logfolder/log_create_storage.log)
			if [[ $ok0 == "1" ]]
				then
					echo -e "$test did not succeed - password is too short - PASS"
				else
					echo -e "$test did not succeed - password is too short - FAIL"
			fi

		else
			#verify logcat
			ok1=$(adb logcat -d > $logfolder/log_create_storage.log && grep -c 'Secure storage created for '$1 $logfolder/log_create_storage.log)
			#ENCRYPTED_STORAGE_PATH should contain the encrypted files
			ok2=$(adb shell ls $3 | grep -c ECRYPTFS)
			ok3=$(adb shell ls -la /data/misc/keystore/ | grep -c $KEY)  
			#if [[ $ok1 == "1" && $ok2 == "3" && $ok3 == "1" ]]		#if the hardcoded KEY is used ok3 is needed
			if [[ $ok1 == "1" && $ok2 == "3" ]]
			then
				echo -e "$test - PASS"
			else
				echo -e "$test - FAIL"
			fi
	fi
	adb logcat -c
}

function check_labels() {
	test='Verify SeLinux attributes'
	labela=$(adb shell 'ls -lZ '$1'' | grep 'u:test_a:test_a:s0')
	labelb=$(adb shell 'ls -lZ '$1'' | grep 'u:test_b:test_b:s0')
	labelc=$(adb shell 'ls -lZ '$1'' | grep 'u:test_c:test_c:s0')
	if [[ -n $labela && -n $labelb && -n $labelc ]]
		then
			echo -e "$test - PASS"
		else
			echo -e "$test - FAIL"
	fi
	adb logcat -c
}

function unlock_storage() {
	test='Unlock secure container '$1
	adb shell efs-tools storage unlock $1 $2
	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_unlock_storage.log && grep -c 'Secure storage '$1' unlocked' $logfolder/log_unlock_storage.log)
	#$STORAGE_PATH should contain the files in plain text
	ok2=$(adb shell ls $1| grep -c txt)
	if [[ $ok1 == "1" && $ok2 == "3" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
	adb logcat -c
}

function lock_storage() {
	test='Lock secure container '$1
	adb shell efs-tools storage lock $1
	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_lock_storage.log && grep -c 'Secure storage '$1' locked' $logfolder/log_lock_storage.log)

	#$STORAGE_PATH should be empty
	ok2=$(adb shell ls $1 | grep -c txt)

	if [[ $ok1 == "1" && $ok2 == "0" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
	adb logcat -c
}

function change_passwd() {
	test='Change container password '$1
	#cleaning the logcat again
	adb logcat -c
	adb shell efs-tools storage change_passwd $1 $2 $3
      	if [[ `expr length $3` -le "3" ]]
		then
			ok0=$(adb logcat -d >  $logfolder/log_change_passwd.log && grep -c 'New passwd too short' $logfolder/log_change_passwd.log)
			if [[ $ok0 == "1" ]]
				then
					echo -e "$test did not succeed - password is too short - PASS"
				else
					echo -e "$test did not succeed - password is too short - FAIL"
			fi

		else
			#verify logcat
			ok1=$(adb logcat -d >  $logfolder/log_change_passwd.log && grep -c 'Change passwd successful for '$1' storage' $logfolder/log_change_passwd.log)
			if [[ $ok1 == "1" ]]
			then
				echo -e "$test from: $2 to: $3 - PASS"
			else
				echo -e "$test from: $2 to: $3 - FAIL"
		fi
        fi
	adb logcat -c
}

function restore_storage() {
	test='Decrypt content and remove secure container '$1
	
	adb shell efs-tools storage restore $1 $2

	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_restore_storage.log && grep -c 'Secure storage '$1' removed' $logfolder/log_restore_storage.log)
	ok2=$(adb logcat -d >  $logfolder/log_restore_storage.log && grep -c 'Secure storage '$1' restored' $logfolder/log_restore_storage.log)

	#ENCRYPTED_STORAGE_PATH folder does not exist anymore
	ok3=$(adb shell ls $ENCRYPTED_STORAGE_PATH | grep -c ECRYPTFS)
	#STORAGE_PATH folder contains the files in plaintext
	ok4=$(adb shell ls $1 | grep -c txt)

	if [[ $ok1 == "1" && $ok2 == "1" && $ok3 == "0" && $ok4 == "3" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
	adb logcat -c
}

function remove_storage() {
	test='Remove secure container'
	adb shell efs-tools storage remove $1
	#keys are no more present in /data/misc/keystore
	ok1=$(adb shell ls -la /data/misc/keystore/ | grep -c $KEY)  
	#verify logcat
	ok2=$(adb logcat -d >  $logfolder/log_remove_storage.log && grep -c 'Secure storage '$1' removed' $logfolder/log_remove_storage.log)
	#STORAGE_PATH and ENCRYPTED_STORAGE_PATH folders are now removed/empty
	ok3=$(adb shell ls $3 | grep -c ECRYPTFS)
	ok4=$(adb shell ls $1 | grep -c txt)
	#if [[ $ok1 == "0" && $ok2 == "1" && $ok3 == "0" && $ok4 == "0" ]]   #if the hardcoded KEY is used ok1 is needed
	if [[ $ok2 == "1" && $ok3 == "0" && $ok4 == "0" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
}


#EDC Functions
function edc_create_storage() {
# "Usage: efs create <storage_path> <passwd>"
	test='CREATE a libefs '$1' container with edc'
	adb shell edc efs-server create $1 $2 > /dev/null
	if [[ `expr length $2` -le "3" ]]
		then
			ok0=$(adb logcat -d > $logfolder/log_edc_create_storage.log && grep -c 'Passwd too short' $logfolder/log_edc_create_storage.log)
			if [[ $ok0 == "1" ]]
				then
					echo -e "$test did not succeed - password is too short - PASS"
				else
					echo -e "$test did not succeed - password is too short - FAIL"
			fi

		else
			#verify logcat
			ok1=$(adb logcat -d > $logfolder/log_edc_create_storage.log && grep -c 'Secure storage created for '$1 $logfolder/log_edc_create_storage.log)
			#ENCRYPTED_STORAGE_PATH should contain the encrypted files
			ok2=$(adb shell ls $3 | grep -c ECRYPTFS)
			ok3=$(adb shell ls -la /data/misc/keystore/ | grep -c $KEY)  
			#if [[ $ok1 == "1" && $ok2 == "3" && $ok3 == "1" ]]		#if the hardcoded KEY is used ok3 is needed
			if [[ $ok1 == "1" && $ok2 == "3" ]]
			then
				echo -e "$test - PASS"
			else
				echo -e "$test - FAIL"
			fi
	fi
	adb logcat -c

}

function edc_unlock_storage() {
# "Usage: efs unlock <storage_path> <passwd>"
	test='UNLOCK the libefs '$1' container with EDC'
	adb shell edc efs-server unlock $1 $2 > /dev/null
	#verify logcat	
	ok1=$(adb logcat -d >  $logfolder/log_edc_unlock_storage.log && grep -c 'Secure storage '$1' unlocked' $logfolder/log_edc_unlock_storage.log)
	#$STORAGE_PATH should contain the files in plain text
	ok2=$(adb shell ls $1| grep -c txt)
	if [[ $ok1 == "1" && $ok2 == "3" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
	adb logcat -c
}

function edc_lock_storage() {
# "Usage: efs lock <storage_path>" 
	test='LOCK the libefs '$1' container with EDC'
	adb shell edc efs-server lock $1 > /dev/null

	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_edc_lock_storage.log && grep -c 'Secure storage '$1' locked' $logfolder/log_edc_lock_storage.log)

	#$STORAGE_PATH should be empty
	ok2=$(adb shell ls $1 | grep -c txt)

	if [[ $ok1 == "1" && $ok2 == "0" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
	adb logcat -c


}

function edc_change_passwd() {
# "Usage: efs change_passwd <storage_path> <old_passwd> <new_passwd>"
	test='CHANGE PASSWORD for the libefs '$1' container with EDC'
	adb shell edc efs-server change_passwd $1 $2 $3 > /dev/null
      	if [[ `expr length $3` -le "3" ]]
		then
			ok0=$(adb logcat -d >  $logfolder/log_edc_change_passwd.log && grep -c 'New passwd too short' $logfolder/log_edc_change_passwd.log)
			if [[ $ok0 == "1" ]]
				then
					echo -e "$test did not succeed - password is too short - PASS"
				else
					echo -e "$test did not succeed - password is too short - FAIL"
			fi

		else
			#verify logcat
			ok1=$(adb logcat -d >  $logfolder/log_edc_change_passwd.log && grep -c 'Change passwd successful for '$1' storage' $logfolder/log_edc_change_passwd.log)
			if [[ $ok1 == "1" ]]
			then
				echo -e "$test from: $2 to: $3 - PASS"
			else
				echo -e "$test from: $2 to: $3 - FAIL"
		fi
        fi
	adb logcat -c
}

function edc_recover_storage() {
# "Usage: efs recover <storage_path> <password>"
	test='RESTORE the libefs '$1' container with EDC'
	adb shell edc efs-server recover $1 $2 > /dev/null
	#verify logcat
	ok1=$(adb logcat -d >  $logfolder/log_edc_restore_storage.log && grep -c 'Secure storage '$1' removed' $logfolder/log_edc_restore_storage.log)
	ok2=$(adb logcat -d >  $logfolder/log_edc_restore_storage.log && grep -c 'Secure storage '$1' restored' $logfolder/log_edc_restore_storage.log)

	#ENCRYPTED_STORAGE_PATH folder does not exist anymore
	ok3=$(adb shell ls $ENCRYPTED_STORAGE_PATH | grep -c ECRYPTFS)
	#STORAGE_PATH folder contains the files in plaintext
	ok4=$(adb shell ls $1 | grep -c txt)

	if [[ $ok1 == "1" && $ok2 == "1" && $ok3 == "0" && $ok4 == "3" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
	adb logcat -c
}

function edc_remove_storage() {
# "Usage: efs remove <storage_path>"
	test='REMOVE the libefs container with EDC'
	adb shell edc efs-server remove $1 > /dev/null
	#keys are no more present in /data/misc/keystore
	ok1=$(adb shell ls -la /data/misc/keystore/ | grep -c $KEY)  
	#verify logcat
	ok2=$(adb logcat -d >  $logfolder/log_edc_remove_storage.log && grep -c 'Secure storage '$1' removed' $logfolder/log_edc_remove_storage.log)
	#STORAGE_PATH and ENCRYPTED_STORAGE_PATH folders are now removed/empty
	ok3=$(adb shell ls $3 | grep -c ECRYPTFS)
	ok4=$(adb shell ls $1 | grep -c txt)
	#if [[ $ok1 == "0" && $ok2 == "1" && $ok3 == "0" && $ok4 == "0" ]]   #if the hardcoded KEY is used ok1 is needed
	if [[ $ok2 == "1" && $ok3 == "0" && $ok4 == "0" ]]
	then
		echo -e "$test - PASS"
	else
		echo -e "$test - FAIL"
	fi
}

function edc_stat(){
# "Usage: efs stat <storage_path>"
	adb shell edc efs-server stat $1 > /dev/null
}

function edc_encrypt_user_data(){
# "Usage: efs encrypt_user_data <userId> <password>"
	adb shell edc efs-server encrypt_user_data $1 $2 > /dev/null
}

function edc_unlock_user_data(){
# "Usage: efs unlock_user_data <userId> <password>"
	adb shell edc efs-server unlock_user_data $1 $2 > /dev/null
}

function edc_lock_user_data(){
# "Usage: efs lock_user_data <userId>"
	adb shell edc efs-server lock_user_data $1 > /dev/null
}

function edc_change_user_data_passwd(){
# "Usage: efs change_user_data_passwd <userId> <old_passwd> <new_passwd>"
	adb shell edc efs-server change_user_data_passwd $1 $2 $3 > /dev/null
}

function edc_decrypt_user_data(){
# "Usage: efs decrypt_user_data <userId> <password>"
	adb shell edc efs-server decrypt_user_data $1 $2 > /dev/null
}

function edc_remove_user_encrypted_data(){
# "Usage: efs remove_user_encrypted_data <userId>"
	adb shell edc efs remove_user_encrypted_data $1 > /dev/null
}

function edc_user_stat(){
# "Usage: efs user_stat <userId>"
	adb shell edc efs-sever user_stat $1 > /dev/null
}

#~/bin/bash

test_path="/data/data/test"
storage_path="/data/data/.test"
password="password"
property_prefix="efs.encrypt.progress_"
sha_head=10

function progress_test() {
	adb shell "setenforce 0"
	adb shell "rm -rf $test_path $storage_path"
	adb shell "mkdir $test_path"
	adb shell "dd if=/dev/urandom of=$test_path/test.file0 bs=1000000 count=10" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file1 bs=1000000 count=12" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file2 bs=1000000 count=8" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file3 bs=1000000 count=14" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file4 bs=1000000 count=16" &> /dev/null

	property="$property_prefix$(echo -n $test_path | openssl dgst -sha512 | cut -d " " -f2 | head -c $sha_head)"

	adb shell "setprop $property 0"
	if [[ $vold -eq 1 ]]
	then
		adb shell "vdc efs create $test_path $password &" &> /dev/null
	else
		adb shell "efs-tools storage create $test_path $password" &
	fi

	progress=0
	new_progress=0
	total=100
	while [[ $progress -lt $total ]]
	do
		if [[ $vold -eq 1 ]]
		then
			new_progress=$(adb shell "vdc efs get_progress $test_path" | tr -d '\r' | cut -d ' ' -f3)
		else
			new_progress=$(adb shell getprop $property | tr -d '\r')
		fi
		if [[ $new_progress -lt $progress ]]
		then
			result=0
			break
		fi
		progress=$new_progress
	done

	if [[ vold -eq 1 ]]
	then
		adb shell "vdc efs remove $test_path" &> /dev/null
	else
		adb shell "efs-tools storage remove $test_path"
	fi
}

result=1
vold=0
progress_test

if [[ $result -eq 1 ]]
then
	echo "Progress Test - PASS"
else
	echo "Progress Test - FAIL"
fi

result=1
vold=1
progress_test

if [[ $result -eq 1 ]]
then
	echo "VOLD Progress Test - PASS"
else
	echo "VOLD Progress Test - FAIL"
fi


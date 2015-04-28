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
	adb shell "dd if=/dev/urandom of=$test_path/test.file0 bs=1000000 count=2" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file1 bs=1000000 count=6" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file2 bs=1000000 count=10" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file3 bs=1000000 count=14" &> /dev/null
	adb shell "dd if=/dev/urandom of=$test_path/test.file4 bs=1000000 count=18" &> /dev/null

	property="$property_prefix$(echo -n $test_path | openssl dgst -sha512 | cut -d " " -f2 | head -c $sha_head)"

	adb shell "setprop $property 0"
	if [[ $edc -eq 1 ]]
	then
		adb shell "edc efs-server create $test_path $password" &> /dev/null
	else
		adb shell "efs-tools storage create $test_path $password" &
	fi

	progress=0
	new_progress=0
	total=100
	while [[ $progress -lt $total ]]
	do
		if [[ $edc -eq 1 ]]
		then
			new_progress=$(adb shell "edc efs-server get_progress $test_path" | tr -d '\r' | cut -d ' ' -f3)
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

	if [[ $edc -eq 1 ]]
	then
		adb shell "edc efs-server remove $test_path" &> /dev/null
	else
		adb shell "efs-tools storage remove $test_path"
	fi

        adb shell "rm -rf $test_path"
}

result=1
edc=1
echo -n "EDC Progress Test "
progress_test

if [[ $result -eq 1 ]]
then
	echo "- PASS"
else
	echo "- FAIL"
fi

result=1
edc=0
echo -n "Progress Test "
progress_test

if [[ $result -eq 1 ]]
then
	echo "- PASS"
else
	echo "- FAIL"
fi


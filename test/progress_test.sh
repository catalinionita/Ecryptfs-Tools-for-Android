#~/bin/bash

function progress_test() {
	adb shell 'setenforce 0'
	adb shell 'rm -rf /data/data/test /data/data/.test'
	adb shell 'mkdir /data/data/test'
	adb shell 'dd if=/dev/urandom of=/data/data/test/test.file0 bs=1000000 count=10' &> /dev/null
	adb shell 'dd if=/dev/urandom of=/data/data/test/test.file1 bs=1000000 count=12' &> /dev/null
	adb shell 'dd if=/dev/urandom of=/data/data/test/test.file2 bs=1000000 count=8' &> /dev/null
	adb shell 'dd if=/dev/urandom of=/data/data/test/test.file3 bs=1000000 count=14' &> /dev/null
	adb shell 'dd if=/dev/urandom of=/data/data/test/test.file4 bs=1000000 count=16' &> /dev/null
	adb shell 'setprop efs.encrypt.progress "0"'
	adb shell 'efs-tools storage create "/data/data/test/" password' &

	progress=0
	new_progress=0
	total=100
	while [[ $progress -lt $total ]]
		do
		new_progress=$(adb shell getprop "efs.encrypt.progress" | tr -d '\r')
		if [[ $new_progress -lt $progress ]]
		then
			result=0
			break
		fi
		progress=$new_progress
	done
}

result=1
progress_test

if [[ $result -eq 1 ]]
then
	echo "Progress Test - PASS"
else
	echo "Progress Test - FAIL"
fi


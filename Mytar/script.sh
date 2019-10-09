#! /bin/bash
#Jesus Martin Moraleda 06277633J
#Jorge Arevalo Echevarria 51000180A


if test -e mytar -a -x mytar; then
	echo "mytar exists and it is executable"
	
	if test -e tmp; then
		echo "tmp found. Deleting..."
		rm -r tmp
	fi

		echo "Creating new tmp directory"
	mkdir tmp
	cd ./tmp

	echo "Creating new archives..."

	echo "Archive file1.txt"
	echo "Hello World!" > file1.txt

	echo "Archive file2.txt"
	head -n 10 /etc/passwd > file2.txt

	echo "Archive file3.dat"
	head -c 1024 /dev/urandom > file3.dat

	echo "Compressing files in filetar.mtar"
	./../mytar -c -f filetar.mtar file1.txt file2.txt file3.dat

	mkdir out 
	cp filetar.mtar ./out/	
	cd ./out

	./../../mytar -x -f filetar.mtar 
	echo "Extracting content.."
	cd ..

	echo "Checking differences between file1" 
	
 	if !(diff file1.txt ./out/file1.txt); then 
		echo diff -q file1.txt ./out/file1.txt
		exit 1
	else
		echo "Yes! Files are the same!"

		echo  "Checking differences between file2"

		if !(diff file2.txt ./out/file2.txt);then 
			echo diff -q file2.txt ./out/file2.txt
			exit 1
		else
			echo "Yes! Files are the same!"

			echo "Checking differences between file3"
			if !(diff file3.dat ./out/file3.dat); then
				echo diff -q file3.dat ./out/file3.dat
				exit 1
			else
				echo "Yes! Files are the same!"
				cd ../..
				echo "Correct!"
				exit 0
			fi
		fi
	fi

else
	echo "mytar doesnt exit!"
	exit 1
fi

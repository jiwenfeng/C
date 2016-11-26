all:
	cd algorithm && make
	cd list && make
	cd queue && make
	cd heap && make
	cd bitmap && make
	cd memory && make
	cd tree && make


clean:
	cd algorithm && make clean
	cd list && make clean
	cd queue && make clean
	cd heap && make clean
	cd bitmap && make clean
	cd memory && make clean
	cd tree && make clean

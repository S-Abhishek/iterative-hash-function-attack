I. Necessary apt packages (linux)
	1. libopenmp-dev (For multithreading)
	2. libssl-dev    (For Crypto functions)

II. Compilation flags
g++ file.cpp -lssl -lcrypto -fopenmp

III. Set 'n' number of threads
	export OMP_NUM_THREADS=n

IV. Steps
	1. Create a folder data in working directory (codes)
	
	2. Generate random 2^K hashes
		a. g++ msg_generator.cpp
		b. ./a.out
		
	2. Build Diamond structure
		a. g++ diamond_tree.cpp -lssl -lcrypto -fopenmp
		b. ./a.out
		
	3. Herd a message
		a. Place message in the msg variable in the herd_message.cpp file
		b. g++ herd_message.cpp -lssl -lcrypto -fopenmp
		c. ./a.out

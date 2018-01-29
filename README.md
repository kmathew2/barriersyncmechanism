# barriersyncmechanism
Barrier mechanisms exists in Linux kernel to synchronize the iterations for multiple threads and processes. 
A new syscall (system call) was implemented using c programming in linux to implement the barrier mechanism. 
The number of barriers were configured as two for the testing and different child processes were created each containing two barriers. 
The number of threads and the barrier timeout is retrieved from the user. 
Also, there was an option added to not configure the timeout for any barrier.

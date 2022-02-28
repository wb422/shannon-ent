#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <signal.h>

#include <unistd.h>

#include <sys/stat.h>
#include <fcntl.h>

static volatile int keep_reading = 1;

void sigint_handle(int n) {
	keep_reading = 0;
}

int main(int argc, char *argv[]) {

	if (argc < 2) {
		fputs("Usage: shannon-entropy <file>\n", stderr);
		return 1;
	}

	int fd = open(argv[1], O_RDONLY);
	
	struct stat file_stat = { 0 };
	if (fstat(fd, &file_stat) != 0 ) {
		fputs("Could not stat file\n", stderr);
		return 1;
	}
	
	signal(SIGINT, sigint_handle);

	uint64_t count_array[256]  = { 0 };
	uint8_t  byte_buffer[4096] = { 0 };
	ssize_t  bytes_read        = 0;
	size_t   total_read        = 0;

	bytes_read = read(fd, byte_buffer, 4096);
	while (bytes_read > 0 && keep_reading) {
		for(ssize_t offset = 0; offset < bytes_read; ++offset) {
			count_array[byte_buffer[offset]] += 1;
		}
		total_read += bytes_read;
		bytes_read = read(fd, byte_buffer, 4096);
	}
	
	double frequency_array[256] = { 0 };
	for (size_t i = 0; i < 256; ++i) {
		if (count_array[i] != 0) 
			frequency_array[i] = (double) count_array[i] / total_read;
	}
	
	// Calculate entropy from frequency array
	
	double entropy = 0.0;
	for (size_t i = 0; i < 256; ++i) {
		if(frequency_array[i] != 0)
			entropy += frequency_array[i] * log2(frequency_array[i]);
	}
	
	entropy = -entropy;
	
	printf("Shannon Entropy (%u bytes): %.16f\n", total_read, entropy);
	
	return 0;
	
}

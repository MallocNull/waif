#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#define NULL 0
#define BOOL uint8_t
#define TRUE 1
#define FALSE 0

#define SAMPLE_RATE	48000
#define BYTE_RATE 800

/* CMD LINE ARG STRUCT */
struct {
	char *source, *destination;
	BOOL encoding;
} args;

/* FILE IO */
FILE *inFile, *outFile;
void writeWavHeader(uint32_t inputFileSize);
void writeBytes(uint64_t bytes, int byteCount) {
	for (int i = 0; i < byteCount; ++i)
		fputc(((char*)bytes)[7-byteCount+i+1], outFile);
}

/* MATH */
double freqPeriodConv(double fop) {
	return 1.f / fop;
}

double samplePeriod() {
	return freqPeriodConv(SAMPLE_RATE);
}

int samplesPerByte() {
	return SAMPLE_RATE / BYTE_RATE;
}

uint32_t audioFileSize(uint32_t inputFileSize) {
	return samplesPerByte() * inputFileSize * 2;
}

/* MISC */
void printHelp();

int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 4) {
		printHelp();
		return -1;
	}

	args.source = NULL;
	args.destination = "wout.raw";
	args.encoding = TRUE;
	for (int i = 0; i < argc; ++i) {
		switch (i) {
		case 0:
			if (strlen(argv[i]) == 2 && argv[i][0] == '/') {
				if (argv[i][1] == 'D' || argv[i][1] == 'd')
					args.encoding = FALSE;
				else if (argv[i][1] != 'E' && argv[i][1] != 'e') {
					printHelp();
					return -1;
				}
				break;
			}
		default:
			if (args.source == NULL)
				args.source = argv[i];
			else if (args.destination == NULL)
				args.destination = argv[i];
			else {
				printHelp();
				return -1;
			}
			break;
		}
	}

	if ((inFile = fopen(args.source, "rb")) == NULL) {
		printf("Source file does not exist or cannot be accessed.");
		return -1;
	}

	if ((outFile = fopen(args.destination, "wb+")) == NULL) {
		printf("Destination file cannot be written to.");
		return -1;
	}

	if (args.encoding == TRUE) {
		fseek(inFile, 0, SEEK_END);
		uint32_t fileSize = ftell(inFile);
		rewind(inFile);

		writeWavHeader(fileSize);
		double dt = samplePeriod();
		int spb = samplesPerByte();
		for(;;) {

		}
	}

	fclose(inFile);
	fclose(outFile);
	return 0;
}

void printHelp() {
	printf("Waif - Wireless Audio Interchange Format converter\n\n");
	printf("waif [/E] [/D] source [destination]\n\n");
	printf("  source\tSpecifies the file to be converted.\n");
	printf("  destination\tSpecifies the directory and/or filename\n  \t\tfor the converted file.\n");
	printf("  /E\t\tEncodes the file.\n");
	printf("  /D\t\tDecodes the file.\n\n");
	printf("The /E and /D switches may not be used together. If neither\nis specified, /E is implied. If destination is not supplied,\noutput will be written to 'wout'.");
}

void writeWavHeader(uint32_t inputFileSize) {
	uint32_t dataSize = samplesPerByte() * inputFileSize * 2;
	fputs("RIFF", outFile);
	writeBytes(36 + dataSize, 4);
	fputs("WAVE", outFile);
	fputs("fmt ", outFile);
	writeBytes(16, 4);
	writeBytes(1, 2);
	writeBytes(1, 2);
	writeBytes(SAMPLE_RATE, 4);
	writeBytes(SAMPLE_RATE * 2, 4);
	writeBytes(2, 2);
	writeBytes(16, 2);
	fputs("data", outFile);
	fputs("fmt ", outFile);
	writeBytes(dataSize, 4);
}
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#define NULL 0
#define BOOL uint8_t
#define TRUE 1
#define FALSE 0

#define SAMPLE_RATE	48000
#define BYTE_RATE 1000

/* CMD LINE ARG STRUCT */
struct {
	char *source, *destination;
	BOOL encoding;
} args;

/* FILE IO */
FILE *inFile, *outFile;
void writeWavHeader(uint32_t inputFileSize);
void writeBytes(uint64_t bytes, int byteCount) {
	for (int i = 0; i < byteCount; ++i) {
		int offset = 8 * (byteCount - i - 1);
		uint64_t byte	= bytes & (0xFFll << offset);
		uint8_t bytesh	= byte >> offset;
		fputc(bytesh, outFile);
	}
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

/* MISC */
void printHelp();

int main(int argc, char *argv[]) {
	if (argc < 2 || argc > 4) {
		printf("a\n");
		printHelp();
		return -1;
	}

	args.source = NULL;
	args.destination = "wout.raw";
	args.encoding = TRUE;
	for (int i = 1; i < argc; ++i) {
		switch (i) {
		case 1:
			if (strlen(argv[i]) == 2 && argv[i][0] == '/') {
				if (argv[i][1] == 'D' || argv[i][1] == 'd')
					args.encoding = FALSE;
				else if (argv[i][1] != 'E' && argv[i][1] != 'e') {
					printf("b\n");
					printHelp();
					return -1;
				}
				break;
			}
		default:
			if (args.source == NULL)
				args.source = argv[i];
			else
				args.destination = argv[i];
			break;
		}
	}

	if ((inFile = fopen(args.source, "rb")) == NULL) {
		printf("Source file does not exist or cannot be accessed.\n");
		return -1;
	}

	if ((outFile = fopen(args.destination, "wb+")) == NULL) {
		printf("Destination file cannot be written to.\n");
		return -1;
	}

	if (args.encoding == TRUE) {
		fseek(inFile, 0, SEEK_END);
		uint32_t fileSize = ftell(inFile);
		rewind(inFile);

		writeWavHeader(fileSize);
		double t = 0;
		double dt = samplePeriod();
		int spb = samplesPerByte();
		for(;;) {
			uint8_t byte = fgetc(inFile);
			if (feof(inFile))
				break;

			double amplitude = byte == 0 ? 0 : 128 * byte - 64;
			for (int i = 0; i < spb; ++i) {
				int pt = i < spb/2 ? amplitude : -amplitude;//(int)round(amplitude * sin(2.f*3.14159*BYTE_RATE * t));
				writeBytes(pt, 2);
				t += dt;
			}
		}
	}
	else {
		int spb = samplesPerByte();
		fseek(inFile, 36, SEEK_SET);

		for (;;) {
			int16_t max = 0;
			for (int i = 0; i < spb; ++i) {
				uint8_t msb = fgetc(inFile);
				uint8_t lsb = fgetc(inFile);
				if (feof(inFile))
					break;

				int16_t inval = (msb << 8) | lsb;
				max = inval > max ? inval : max;
			}

			int value;
			for (value = 0; value < 256; ++value) {
				if (value == 0) {
					if (max >= 0 && max <= 32)
						break;
				} else if (value == 1) {
					if (max > 32 && max <= 128)
						break;
				} else {
					if (max > (value - 1) * 128 && max <= value * 128)
						break;
				}
			}

			fputc(value, outFile);

			if (feof(inFile))
				break;
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
	printf("The /E and /D switches may not be used together. If neither\nis specified, /E is implied. If destination is not supplied,\noutput will be written to 'wout.raw'.\n");
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
	writeBytes(dataSize, 4);
}
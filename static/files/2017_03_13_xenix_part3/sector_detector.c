// Sector Detector - Finds Missing Sectors in XENIX 386 2.2.3 floppies
// Written by - Michael Casadevall <mcasadevall@ubuntu.com> 2/26/17

// This code is licensed under the CC0 license and may be used for any purpose
// https://creativecommons.org/publicdomain/zero/1.0/legalcode

// Changelog
//   0.1 (2/26/17) - Initial Release

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    char missing_sector[512];
    char curr_sector[512];

    FILE * disk_image;

    if (argc == 1) {
        printf("sector_detector: DISK1 DISK2 ...\n");
        return -1;
    }

    memset(missing_sector, 0xF6, 512);

    for (int arg = 1; arg != argc; arg++) {
        printf("%s:\n", argv[arg]);

        disk_image = fopen(argv[arg], "rb");
        if (disk_image == 0) {
            perror("fopen: ");
            return -1;
        }

        int sectors_to_read = 1440; /* 737280/512 */
        int first_missing_sector = 0;
        int linear_sectors_missing = 0;

        for (int i = 1; i < sectors_to_read; i += 1) {
            fseek(disk_image, (i-1)*512, SEEK_SET);
            int read = fread(curr_sector, 512, 1, disk_image);
            if (read != 1) {
                printf("read error, got %d bytes\n", read);
                return -1;
            }

            if (feof(disk_image) != 0) {
                printf("feof hit\n");
                break;
            }

            if (memcmp(curr_sector, missing_sector, 512) == 0) {
                /* Sector missing */
                //printf("sec: %d\n", i);
                if (first_missing_sector == 0) {
                    first_missing_sector = i;
                    linear_sectors_missing = 1;
                } else {
                    linear_sectors_missing++;
                }
            } else { /* Sector found, report any missing before */
                if (first_missing_sector != 0) {
                    if (linear_sectors_missing == 1) {
                        printf("sector %d (%x)is missing\n", 
                                first_missing_sector, 
                                (first_missing_sector-1)*512);
                    } else {
                        printf("sector %d (%x) to %d (%x) is missing\n",
                            first_missing_sector, 
                            (first_missing_sector-1)*512, 
                            linear_sectors_missing+first_missing_sector-1,
                            (linear_sectors_missing+first_missing_sector-1)*512);
                    }
                    linear_sectors_missing = 0;
                    first_missing_sector = 0;
                }
            }
        }

        // Final tally of sectors from end of disk
        if (first_missing_sector != 0) {
            // Need to knock off one 
            printf("%d sectors at end empty, starting at %d\n", linear_sectors_missing, first_missing_sector);
        }

        // Put some lines at the end for > output
        printf("\n");

        fclose(disk_image);
    }

    return 0;
}

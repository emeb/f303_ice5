/*
 * cmd.c - Command parsing routines for STM32F303 breakout SPI to ice5 FPGA
 * 05-11-16 E. Brombaugh
 */
#include "stm32f30x.h"
#include "arm_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "arm_math.h"
#include "usart.h"
#include "cyclesleep.h"
#include "ice5.h"
#include "ff.h"

#define MAX_ARGS 4

/* locals we use here */
char cmd_buffer[256];
char *cmd_wptr;
const char *cmd_commands[] = 
{
	"help",
	"spi_read",
	"spi_write",
	"dir",
    "config_file",
    ""
};

/* Fat FS stuff */
FRESULT fres;
FATFS Fatfs, *fs;			/* File system object for each logical drive */
FIL File;					/* File object */
DIR Dir;					/* Directory object */
FILINFO Finfo;

/* reset buffer & display the prompt */
void cmd_prompt(void)
{
	/* reset input buffer */
	cmd_wptr = &cmd_buffer[0];

	/* prompt user */
	printf("\rCommand>");
}

/* process command line after <cr> */
void cmd_proc(void)
{
	char *token, *argv[MAX_ARGS];
	int argc, cmd, reg;
	unsigned long data;

	/* parse out three tokens: cmd arg arg */
	argc = 0;
	token = strtok(cmd_buffer, " ");
	while(token != NULL && argc < MAX_ARGS)
	{
		argv[argc++] = token;
		token = strtok(NULL, " ");
	}

	/* figure out which command it is */
	if(argc > 0)
	{
		cmd = 0;
		while(cmd_commands[cmd] != '\0')
		{
			if(strcmp(argv[0], cmd_commands[cmd])==0)
				break;
			cmd++;
		}
	
		/* Can we handle this? */
		if(cmd_commands[cmd] != '\0')
		{
			printf("\r\n");

			/* Handle commands */
			switch(cmd)
			{
				case 0:		/* Help */
					printf("help - this message\r\n");
					printf("spi_read <addr> - FPGA SPI read reg\r\n");
					printf("spi_write <addr> <data> - FPGA SPI write reg, data\r\n");
					printf("dir - directory of SD card root\r\n");
					printf("config_file <file> - Configure FPGA from <file>\r\n");
					break;
	
				case 1: 	/* spi_read */
					if(argc < 2)
						printf("spi_read - missing arg(s)\r\n");
					else
					{
						reg = (int)strtoul(argv[1], NULL, 0) & 0x7f;
						ICE5_FPGA_Slave_Read(reg, &data);
						printf("spi_read: 0x%02X = 0x%08lX\r\n", reg, data);
					}
					break;
	
				case 2: 	/* spi_write */
					if(argc < 3)
						printf("spi_write - missing arg(s)\r\n");
					else
					{
						reg = (int)strtoul(argv[1], NULL, 0) & 0x7f;
						data = strtoul(argv[2], NULL, 0);
						ICE5_FPGA_Slave_Write(reg, data);
						printf("spi_write: 0x%02X 0x%08lX\r\n", reg, data);
					}
					break;
		
				case 3: 	/* dir */
                    /* Open the root directory */
                    fres = f_opendir(&Dir, "\\");
                    printf("opened directory /: 0x%x\n", fres);
                    
                    if(fres == 0)
                    {
                        /* dump top-level directory */
                        int32_t p1 = 0;
                        uint32_t s1 = 0, s2 = 0;
                        for(;;) {
                            fres = f_readdir(&Dir, &Finfo);
                            if ((fres != FR_OK) || !Finfo.fname[0]) break;
                            if (Finfo.fattrib & AM_DIR) {
                                s2++;
                            } else {
                                s1++; p1 += Finfo.fsize;
                            }
                            printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9lu  %s\n",
                                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                                (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                                (Finfo.fdate >> 9) + 1980, 
                                (Finfo.fdate >> 5) & 15,
                                Finfo.fdate & 31,
                                (Finfo.ftime >> 11),
                                (Finfo.ftime >> 5) & 63,
                                Finfo.fsize,
                                &(Finfo.fname[0]));
                        }
                        printf("%4u File(s),%10lu bytes total\n%4u Dir(s)\n",
                            (unsigned int)s1, p1, (unsigned int)s2);
                        //if (f_getfree("\\", (DWORD*)&p1, &fs) == FR_OK)
                        //	printf(", %10lu bytes free\n", p1 * fs->csize * 512);
                    }
					break;
		
				case 4: 	/* config_file */
					if(argc < 2)
						printf("config_file - missing filename argument\r\n");
					else
					{
                        /* try to open the file */
                        fres = f_open(&File, argv[1], FA_READ);
                        if(!fres)
                        {
                            /* try to configure FPGA */
                            uint8_t result = ICE5_FPGA_Config_File(&File);
                            
                            if(result)
                                printf("config_file - ICE5_FPGA_Config_File returned %d\r\n", result);
                        }
                        else
                        {
                            printf("config_file - can't open file %s\r\n", argv[1]);
                        }
                    }
                    break;
                
				default:	/* shouldn't get here */
					break;
			}
		}
		else
			printf("Unknown command\r\n");
	}
}
	
void init_cmd(void)
{
    /* mount the SD card */
    fres = f_mount(0, &Fatfs);
    printf("mounted drive 0: %x\n", fres);
                    
	/* prompt */
	cmd_prompt();
}

void cmd_parse(char ch)
{
	/* accumulate chars until cr, handle backspace */
	if(ch == '\b')
	{
		/* check for buffer underflow */
		if(cmd_wptr - &cmd_buffer[0] > 0)
		{
			printf("\b \b");		/* Erase & backspace */
			cmd_wptr--;		/* remove previous char */
		}
	}
	else if(ch == '\r')
	{
		*cmd_wptr = '\0';	/* null terminate, no inc */
		cmd_proc();
		cmd_prompt();
	}
	else
	{
		/* check for buffer full (leave room for null) */
		if(cmd_wptr - &cmd_buffer[0] < 254)
		{
			*cmd_wptr++ = ch;	/* store to buffer */
			putc(ch, stdout);			/* echo */
		}
	}
	fflush(stdout);
}

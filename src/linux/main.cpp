/***************************************************************************

    file                 : main.cpp
    created              : Sat Mar 18 23:54:30 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: main.cpp,v 1.14.2.3 2012/06/01 01:59:42 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>

#include <GL/glut.h>

#include <tgfclient.h>
#include <client.h>

#include "linuxspec.h"
#include <raceinit.h>
#include <stdio.h>

//#include <semaphore.h>
//#include <sys/types.h>
//#include <sys/mman.h>
//#include <sys/stat.h>
//#include <fcntl.h>
#include <sys/shm.h>

extern bool bKeepModules;

shared_memory_info_st* shared_memory_info = NULL;

//void startMemorySharing(int memid) {
//	char name[NAME_MAX];
//	sprintf(name, "/torcs-%d", memid);
//
//	int fd_shm = shm_open(name, O_RDWR | O_CREAT, 0660);
//	if (fd_shm == -1) {
//		fprintf(stderr, "Failed to open shared memory");
//		exit(EXIT_FAILURE);
//	}
//
//	if (ftruncate(fd_shm, sizeof(struct shared_use_st)) == -1) {
//		fprintf(stderr, "Failed to open shared memory");
//		exit(EXIT_FAILURE);
//	}
//
//	shared_memory = (shared_use_st*) mmap(NULL, sizeof(shared_use_st),
//			PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
//	if (shared_memory == MAP_FAILED) {
//		fprintf(stderr, "Failed to open shared memory");
//		exit(EXIT_FAILURE);
//	}
//	printf("Started shared memory at %s\n", name);
//
//}

void startMemorySharingSys5(int memid) {
	int xw = getScreenW(), yw = getScreenH();
	shared_memory_info = (shared_memory_info_st*)malloc(sizeof(shared_memory_info_st));
	shared_memory_info->memid = memid;
	int shmid;
	// establish memory sharing
	shmid = shmget((key_t) memid, xw * yw * 3 * sizeof(unsigned char),
			0666 | IPC_CREAT);
	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	shared_memory_info->shmid = shmid;
	void* shm = shmat(shmid, 0, 0);
	if (shm == (void*) -1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}

	shared_memory_info->image_data = (unsigned char*) shm;
	printf("Shared memory image size (%d, %d), memid: %d\n", xw, yw, memid);

}



static void
init_args(int argc, char **argv, const char **raceconfig)
{
	int i;
	char *buf;
    
    setNoisy(false);
    setVersion("2013");

	i = 1;

	while(i < argc) {
		if(strncmp(argv[i], "-l", 2) == 0) {
			i++;

			if(i < argc) {
				buf = (char *)malloc(strlen(argv[i]) + 2);
				sprintf(buf, "%s/", argv[i]);
				SetLocalDir(buf);
				free(buf);
				i++;
			}
		} else if(strncmp(argv[i], "-L", 2) == 0) {
			i++;

			if(i < argc) {
				buf = (char *)malloc(strlen(argv[i]) + 2);
				sprintf(buf, "%s/", argv[i]);
				SetLibDir(buf);
				free(buf);
				i++;
			}
		} else if(strncmp(argv[i], "-D", 2) == 0) {
			i++;

			if(i < argc) {
				buf = (char *)malloc(strlen(argv[i]) + 2);
				sprintf(buf, "%s/", argv[i]);
				SetDataDir(buf);
				free(buf);
				i++;
			}
		} else if(strncmp(argv[i], "-s", 2) == 0) {
			i++;
			SetSingleTextureMode();
		} else if (strncmp(argv[i], "-t", 2) == 0) {
		    i++;
		    if (i < argc) {
			long int t;
			sscanf(argv[i],"%ld",&t);
			setTimeout(t);
			printf("UDP Timeout set to %ld 10E-6 seconds.\n",t);
			i++;
		    }
		} else if (strncmp(argv[i], "-nodamage", 9) == 0) {
		    i++;
		    setDamageLimit(false);
		    printf("Car damages disabled!\n");
		} else if (strncmp(argv[i], "-nofuel", 7) == 0) {
		    i++;
		    setFuelConsumption(false);
		    printf("Fuel consumption disabled!\n");
		} else if (strncmp(argv[i], "-noisy", 6) == 0) {
		    i++;
		    setNoisy(true);
		    printf("Noisy Sensors!\n");
		} else if (strncmp(argv[i], "-ver", 4) == 0) {
		    i++;
		    if (i < argc) {
					setVersion(argv[i]);
		    		printf("Set version: \"%s\"\n",getVersion());
		    		i++;
		    }
		} else if (strncmp(argv[i], "-nolaptime", 10) == 0) {
		    i++;
		    setLaptimeLimit(false);
		    printf("Laptime limit disabled!\n");   
		} else if (strncmp(argv[i], "-port", 5) == 0) {
			i++;
			if (i < argc) {
				unsigned int port;
				sscanf(argv[i], "%d", &port);
				setScrPort(port);
				printf("UDP port set to %d.\n", port);
				i++;
			}
		}else if (strncmp(argv[i], "-cmdFreq", 8) == 0) {
			i++;
			if (i < argc) {
				unsigned int cmdFreq;
				sscanf(argv[i], "%d", &cmdFreq);
				setRobotCmdFreq(cmdFreq);
				printf("Robot command frequency set to %d.\n", cmdFreq);
				i++;
			}
		}else if (strncmp(argv[i], "-w", 2) == 0) {
			i++;
			if (i < argc) {
				unsigned int screenW;
				sscanf(argv[i], "%d", &screenW);
				setScreenW(screenW);
				printf("Screen width set to %d.\n", screenW);
				i++;
			}
		}else if (strncmp(argv[i], "-h", 2) == 0) {
			i++;
			if (i < argc) {
				unsigned int screenH;
				sscanf(argv[i], "%d", &screenH);
				setScreenH(screenH);
				printf("Screen height set to %d.\n", screenH);
				i++;
			}
		}else if(strncmp(argv[i], "-k", 2) == 0) {
			i++;
			// Keep modules in memory (for valgrind)
			printf("Unloading modules disabled, just intended for valgrind runs.\n");
			bKeepModules = true;
#ifndef FREEGLUT
		} else if(strncmp(argv[i], "-m", 2) == 0) {
			i++;
			GfuiMouseSetHWPresent(); /* allow the hardware cursor */
#endif
		} else if(strncmp(argv[i], "-r", 2) == 0) {
			i++;
			*raceconfig = "";

			if(i < argc) {
				*raceconfig = argv[i];
				i++;
			}

			if((strlen(*raceconfig) == 0) || (strstr(*raceconfig, ".xml") == 0)) {
				printf("Please specify a race configuration xml when using -r\n");
				exit(1);
			}
		} else {
			i++;		/* ignore bad args */
		}
	}

#ifdef FREEGLUT
	GfuiMouseSetHWPresent(); /* allow the hardware cursor (freeglut pb ?) */
#endif
}

/*
 * Function
 *	main
 *
 * Description
 *	LINUX entry point of TORCS
 *
 * Parameters
 *
 *
 * Return
 *
 *
 * Remarks
 *
 */
int
main(int argc, char *argv[])
{
	const char *raceconfig = "";

	init_args(argc, argv, &raceconfig);
	startMemorySharingSys5(getScrPort());

	LinuxSpecInit();			/* init specific linux functions */

	if(strlen(raceconfig) == 0) {
		GfScrInit(argc, argv);	/* init screen */
		TorcsEntry();			/* launch TORCS */
		glutMainLoop();			/* event loop of glut */
	} else {
		// Run race from console, no Window, no OpenGL/OpenAL etc.
		// Thought for blind scripted AI training
		ReRunRaceOnConsole(raceconfig);
	}

	return 0;					/* just for the compiler, never reached */
}


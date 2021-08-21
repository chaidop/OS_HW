#######################################
# 
# Operating Systems Lab
# 
# Project 02 Makefile Template
#
#######################################

# The C compiler
CC = gcc

# Put any flags for the C compiler here
CFLAGS = -Wall -g

# The source files for the Virtual Machine
LIB_SRCS = cpuinit.c cpu.c

# The corresponding object files
LIB_OBJS = cpuinit.o cpu.o

# The scheduler's source files Round Robin
SCHED_SRCS1 = schedule_rr.c
# The scheduler's source files SJF
SCHED_SRCS2 = schedule_sjf.c


# Put the name of any additional source file here
ADD_SRCS = 

# Include files
INC_FILES = privatestructs.h macros.h schedule.h

# The name of the virtual machine's library, without the lib prefix and the .a suffix
LIB_NAME = sched_vm

# The directory where the virtual machine's library resides.
LIB_DIR = .

# The name of the final executable Round Robin
EXEC_NAME1 = rr_sched
# The name of the final executable SJF
EXEC_NAME2 = sjf_sched


all: $(EXEC_NAME1) $(EXEC_NAME2)

$(EXEC_NAME1): $(SCHED_SRCS1) $(INC_FILES) $(LIB_NAME)
	$(CC) $(CFLAGS) $(SCHED_SRCS1) $(ADD_SRCS) -o $(EXEC_NAME1) -l$(LIB_NAME) -L$(LIB_DIR)
	
$(EXEC_NAME2): $(SCHED_SRCS2) $(INC_FILES) $(LIB_NAME)
	$(CC) $(CFLAGS) $(SCHED_SRCS2) $(ADD_SRCS) -o $(EXEC_NAME2) -l$(LIB_NAME) -L$(LIB_DIR)

$(LIB_NAME): $(LIB_SRCS) $(INC_FILES)
	$(CC) -c $(CFLAGS) $(LIB_SRCS)
	ar rcsv lib$(LIB_NAME).a $(LIB_OBJS)


.PHONY: clean

clean:
	rm -v $(EXEC_NAME1) lib$(LIB_NAME).a *.o
	rm -v $(EXEC_NAME2) lib$(LIB_NAME).a *.o


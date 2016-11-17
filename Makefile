all: DPP

DPP: DPP.c                                      
	gcc $^ -o $@ -g -Wall -pthread                                                           

clean:                                                     
	rm -f DPP


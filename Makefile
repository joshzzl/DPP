all: DPP

DPP: DPP.c                                      
	gcc $^ -o $@ -Wall -pthread                                                           

clean:                                                     
	rm -f DPP  


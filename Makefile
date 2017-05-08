
all: up2pa_sim up2pc_sim up2ps_sim

up2pa_sim : up2pa.c up2pa_vc.c up2p.h up2pa.h
	cc -o up2pa_sim up2pa.c up2pa_vc.c -lc -lpthread
	
up2pc_sim : up2pc.c up2pc_vc.c up2pc.h up2p.h
	cc -o up2pc_sim up2pc.c up2pc_vc.c -lc -lpthread

up2ps_sim : up2ps.c up2p.h
	cc -o up2ps_sim up2ps.c -lc -lpthread

clean :
	rm up2pa_sim up2pc_sim up2ps_sim

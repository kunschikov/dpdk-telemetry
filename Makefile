LDFLAGS=-static
CXXFLAGS=-Wall

telemetry-test: dpdk-telemetry.o
	g++ -Wall -o $@ $^ ${LDFLAGS}

.PHONY: clean

clean:
	rm  *.o telemetry-test

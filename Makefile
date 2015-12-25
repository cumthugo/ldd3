
SUBDIRS =  misc-progs misc-modules \
           scull scullc scullp scullv short #skull sculld sbull snull\
	   shortprint pci simple usb tty lddbus

all: subdirs

subdirs:
	for n in $(SUBDIRS); do $(MAKE) -C $$n || exit 1; done

clean:
	for n in $(SUBDIRS); do $(MAKE) -C $$n clean; done

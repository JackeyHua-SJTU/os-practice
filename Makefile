SUBDIRS = ./shell ./sort
COPYDIR = ./copy
MACRO ?= default_value

.PHONY: subdirs $(SUBDIRS) $(COPYDIR)

subdirs: $(SUBDIRS) $(COPYDIR)

$(SUBDIRS):
	$(MAKE) -C $@

$(COPYDIR):
	$(MAKE) -C $@ MACRO=$(MACRO)

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	$(MAKE) -C $(COPYDIR) clean
	
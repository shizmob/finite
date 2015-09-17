DOCDIR    = src
SRCDIR    = src
INCDIR    = include
BINDIR    = bin
OBJDIR    = obj
DESTDIR   =
PREFIX    =
MANPREFIX = $(if $(subst /,,$(PREFIX)),$(PREFIX),/usr)/share/man

D = $(DOCDIR)
S = $(SRCDIR)
I = $(INCDIR)
B = $(if $(BUILD),$(BUILD)/,)$(BINDIR)
O = $(if $(BUILD),$(BUILD)/,)$(OBJDIR)
P = $(DESTDIR)$(PREFIX)
M = $(DESTDIR)$(MANPREFIX)


.PHONY: all clean distclean install uninstall
.SECONDARY:

all:
clean:
	@echo [CLN]
	@rm -rf $(B) $(O)
distclean: clean


$(B) $(O):
	@mkdir -p $@
$(P)/bin $(P)/include $(P)/lib $(P)/sbin $(M)/man1 $(M)/man2 $(M)/man3 $(M)/man4 $(M)/man5 $(M)/man6 $(M)/man7 $(M)/man8:
	@install -d -m 0755 $@

$(P)/bin/% $(P)/sbin/%: $(B)/% | $(P)/bin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@

$(P)/include/%: $(I)/% | $(P)/include
	@echo [INC] $(notdir $@)
	@install -m 0644 $< $@

$(P)/lib/%: $(B)/% | $(P)/lib
	@echo [LIB] $(notdir $@)
	@install -m 0755 $< $@

define man-macro
$$(M)/man$1/%.$1: $$(D)/%.$1 | $$(M)/man$1
	@echo [MAN] $$(notdir $$@)
	@install -m 0644 $$< $$@
endef
$(foreach n,1 2 3 4 5 6 7 8,$(eval $(call man-macro,$(n))))
$(B)/%.so: | $(B)
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $(DYLDFLAGS) $(filter-out %.h,$^) -o $@

$(B)/%.a: | $(B)
	@echo [ LD] $(notdir $@)
	@$(AR) -r $@ $(filter-out %.h,$^) >/dev/null

$(B)/%: | $(B)
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $(filter-out %.h,$^) -o $@

$(O)/%.o: $(S)/%.c | $(O)
	@mkdir -p $(dir $@)
	@echo [ CC] $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $(filter-out %.h,$^) -o $@


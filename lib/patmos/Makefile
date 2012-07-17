
# Base directory where clang is installed (without /bin)
BUILDDIR=../../../patmos-llvm-obj/Debug+Asserts

INSTALLDIR=$(BUILDDIR)

TRIPLE=patmos-unknown-elf

CCBIN=$(BUILDDIR)/bin/clang

AS=$(BUILDDIR)/bin/llvm-as
AR=$(BUILDDIR)/bin/llvm-ar

CC=$(CCBIN) -ccc-host-triple $(TRIPLE) -U__i386__ -D__PATMOS__ -O2 -emit-llvm

-include config.mk

OUTFILES=libll.a libllsyms.o libllsyms.lst

OBJS = divsi3.o modsi3.o udivsi3.o umodsi3.o \
       clzsi2.o \
       muldi3.o divdi3.o moddi3.o udivdi3.o umoddi3.o udivmoddi4.o \
       ashldi3.o ashrdi3.o lshrdi3.o \
       addsf3.o subsf3.o mulsf3.o divsf3.o comparesf2.o \
       adddf3.o subdf3.o muldf3.o divdf3.o comparedf2.o \
       truncdfsf2.o extendsfdf2.o \
       floatsisf.o floatsidf.o floatunsidf.o \
       fixsfsi.o fixdfsi.o fixunsdfsi.o

all: $(OUTFILES)

# TODO split libllsyms.ll into softfloat and non-softfloat, or make two separate versions
libllsyms.lst: libllsyms.ll
	cat $< | grep "^declare" | sed "s/declare .* @\([^(]*\).*$$/\1/" > $@

libll.a: $(addprefix ../,$(OBJS))
	$(AR) r $@ $^

%.o: %.ll
	$(AS) -o $@ $<

clean:
	rm -rf *.o $(OUTFILES) $(addprefix ../,$(OBJS))

install: $(OUTFILES)
	mkdir -p $(INSTALLDIR)/$(TRIPLE)/lib
	cp -v $(OUTFILES) $(INSTALLDIR)/$(TRIPLE)/lib

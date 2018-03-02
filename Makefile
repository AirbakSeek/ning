O = $(CURDIR)/out

CC = gcc

SRCS = \
    src/common/common.c \
    src/core/ning.c \
    src/core/cmd.c \
    src/core/block.c \
    src/core/pack.c \
    src/core/create.c \
    src/core/archive.c \
    src/core/extract.c

SRCS += src/test/test.c

LIBTOMCRYPT_PATH = libs/libtomcrypt
LIBTOMCRYPT_SRCS = \
	$(LIBTOMCRYPT_PATH)/src/misc/crypt/crypt_argchk.c \
	$(LIBTOMCRYPT_PATH)/src/hashes/sha2/sha256.c

LIBTOMCRYPT_FLAGS = -DLTC_NO_TEST

SRCS += $(LIBTOMCRYPT_SRCS)

OBJS = $(addprefix $(O)/,$(SRCS:.c=.o))

DEPS = $(OBJS:.o=.d)

TARGET = ning

CFLAGS = -O2

#LIBS = -lssl
#LDFLAGS = -L/usr/lib

INS = -I$(CURDIR)/src/include
INS += -I$(CURDIR)/$(LIBTOMCRYPT_PATH)/src/headers

CFLAGS += -DLOG_LEVEL=2
CFLAGS += -DAPP_PATH='"$(CURDIR)/"'
CFLAGS += $(LIBTOMCRYPT_FLAGS)
CFLAGS += $(INS)

.PHONY: all $(TARGET) clean

test: $(TARGET)
	rm -rf test_file.ning
	rm -rf test_file.nin
	$(O)/ning -cf test_file
	$(O)/ning -xf test_file.ning
	diff test_file test_file.nin

all: $(TARGET) test

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CC) $^ -o $(O)/$@ $(LDFLAGS) $(LIBS)
	@echo "ning is ready"

clean:
	rm -rf $(O)

$(O)/%.o: %.c
	test -e $(dir $@) || mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@ -MMD -MP -MF $(@:.o=.d)

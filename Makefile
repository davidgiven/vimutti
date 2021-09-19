OBJDIR = .obj

SRCS = \
	src/main.c \
	src/unpackapp.c \
	src/unpackimg.c \
	src/utils.c \

OBJS = $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

vimutti: $(OBJS)
	gcc -Os -g -o $@ $(OBJS)

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	gcc -Os -g -c -o $@ $<


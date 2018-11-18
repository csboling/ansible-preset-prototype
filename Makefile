HEADERS = jsmn/jsmn.h deserialize_jsmn.h ansible_sections.h
OBJECTS = main.o jsmn/jsmn.o deserialize_jsmn.o ansible_sections.o
CC = gcc

default: program

%.o: %.c $(HEADERS)
	$(CC) -c $< -g -o $@

program: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

clean:
	rm -f $(OBJECTS)
	rm -f program


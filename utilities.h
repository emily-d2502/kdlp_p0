#define NPOS (-1)

int find_first_of(const char *str, int (*comp)(int));
char *substr(const char *str, int start, int end);
unsigned int parse_command_line(const char *line, char*** argv_p);

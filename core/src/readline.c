#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

extern const char *gCommands[];

static char* command_generator(const char *text, int state)
{
	const char *name;
	
	static int list_index, len;
	
	if (!state)
	{
		list_index = 0;
		len = strlen (text);
	}
	while (name = gCommands[list_index])
	{
		list_index++;
		if (strncmp (name, text, len) == 0)
			return strdup(name);		
	}
	return ((char *)NULL);
}
char** command_completion (const char *text, int start, int end)
{
	char **matches = NULL;
	char *tmpStr = NULL;
	tmpStr = (char *)text;
	if (start)
	{
		tmpStr = strrchr(text,' ');
		if(tmpStr && strlen(tmpStr) > 1) {
			tmpStr += 1;
		}
		else tmpStr = (char*) text;				
	}
	matches = rl_completion_matches ((const char *)tmpStr, command_generator);		
	return (matches);
}
void initialize_readline ()
{
	rl_attempted_completion_function = command_completion;
}


/** Code is taken from readline library example and modified for the simple
* todo manager driver program 
**/
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <time.h>

/* GNU readline
#include <readline/readline.h>
#include <readline/history.h>
*/
#include <editline/readline.h>

void * xmalloc (size_t size);
void initialize_readline ();
int execute_line (char *line);
int assert_has_value (char *caller, char *arg, char *msg);

typedef int rl_icpfunc_t (char *);

/* The names of functions that actually do the work. */
int com_add_todo (char *);
int com_show_todos (char *);
int com_help(char *);
int com_quit(char *);

/* A structure which contains information on the commands this program
   can understand. */

typedef struct {
   char *name;                   /* User printable name of the function. */
   rl_icpfunc_t *func;           /* Function to call to do the job. */
   char *doc;                    /* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
   { "t", com_add_todo, "Add new TODO item" },
   { "s", com_show_todos, "Show the TODO list" },
   { "?", com_help, "Show list of available commands" },
   { "q", com_quit, "Quit" }, 
   { (char *)NULL, (rl_icpfunc_t *)NULL, (char *)NULL }
};

static struct todo_item {
    char *what;
    struct todo_item *head;
    struct todo_item *next;
} *todo_item_list;


/* Forward declarations. */
char *stripwhite ();
COMMAND *find_command ();

/* The name of this program, as taken from argv[0]. */
char *progname;

/* When non-zero, this means the user is done using this program. */
int done;

char *
dupstr (char* s)
{
   char *r;

   r = xmalloc (strlen (s) + 1);
   strcpy (r, s);
   return (r);
}

int
main (int argc, char **argv)
{
   char *line, *s;

   progname = argv[0];

   setlocale(LC_CTYPE, "");

   initialize_readline();       /* Bind our completer. */

   stifle_history(7);

   /* Loop reading and executing lines until the user quits. */
   for ( ; done == 0; )
   {
      line = readline ("TODO Manager >");

      if (!line)
         break;

      /* Remove leading and trailing whitespace from the line.
         Then, if there is anything left, add it to the history list
         and execute it. */
      s = stripwhite(line);

      if (*s) {

         char* expansion;
         int result;

         result = history_expand(s, &expansion);

         if (result < 0 || result == 2) {
            fprintf(stderr, "%s\n", expansion);
         } else {
            add_history(expansion);
            execute_line(expansion);
         }
         free(expansion);
      }

      free(line);
   }
   exit (0);

   return 0;
}

/* Execute a command line. */
int
execute_line (char *line)
{
   register int i;
   COMMAND *command;
   char *word;

   /* Isolate the command word. */
   i = 0;
   while (line[i] && isspace (line[i]))
      i++;
   word = line + i;

   while (line[i] && !isspace (line[i]))
      i++;

   if (line[i])
      line[i++] = '\0';

   command = find_command (word);

   if (!command)
   {
      fprintf (stderr, "%s: No such command for TODO Manager.\n", word);
      return (-1);
   }

   /* Get argument to command, if any. */
   while (isspace (line[i]))
      i++;

   word = line + i;

   /* Call the function. */
   return ((*(command->func)) (word));
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *
find_command (char *name)
{
   register int i;

   for (i = 0; commands[i].name; i++)
      if (strcmp (name, commands[i].name) == 0)
         return (&commands[i]);

   return ((COMMAND *)NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *
stripwhite (char *string)
{
   register char *s, *t;

   for (s = string; isspace (*s); s++)
      ;

   if (*s == 0)
      return (s);

   t = s + strlen (s) - 1;
   while (t > s && isspace (*t))
      t--;
   *++t = '\0';

   return s;
}

/* **************************************************************** */
/*                                                                  */
/*                  Interface to Readline Completion                */
/*                                                                  */
/* **************************************************************** */

char *command_generator(const char *, int);
char **todoman_completion(const char *, int, int);

/* Tell the GNU Readline library how to complete.  We want to try to
   complete on command names if this is the first word in the line, or
   on filenames if not. */
void
initialize_readline ()
{
   /* Allow conditional parsing of the ~/.inputrc file. */
   rl_readline_name = "TodoMan";

   /* Tell the completer that we want a crack first. */
   rl_attempted_completion_function = todoman_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END
   bound the region of rl_line_buffer that contains the word to
   complete.  TEXT is the word to complete.  We can use the entire
   contents of rl_line_buffer in case we want to do some simple
   parsing.  Returnthe array of matches, or NULL if there aren't any. */
char **
todoman_completion (const char* text, int start, int end)
{
   char **matches;

   matches = (char **)NULL;

   /* If this word is at the start of the line, then it is a command
      to complete.  Otherwise it is the name of a file in the current
      directory. */
   if (start == 0)
      /* TODO */
      matches = completion_matches (text, command_generator);
      /* matches = rl_completion_matches (text, command_generator); */

   return (matches);
}

/* Generator function for command completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == 0), then we start at the top of the list. */
char *
command_generator (text, state)
   const char *text;
   int state;
{
   static int list_index, len;
   char *name;

   /* If this is a new word to complete, initialize now.  This
      includes saving the length of TEXT for efficiency, and
      initializing the index variable to 0. */
   if (!state)
   {
      list_index = 0;
      len = strlen (text);
   }

   /* Return the next name which partially matches from the
      command list. */
   while (name = commands[list_index].name)
   {
      list_index++;

      if (strncmp (name, text, len) == 0)
         return (dupstr(name));
   }

   /* If no names matched, then return NULL. */
   return ((char *)NULL);
}

/* **************************************************************** */
/*                                                                  */
/*                       TODOMan Commands                           */
/*                                                                  */
/* **************************************************************** */

static void init_todo(struct todo_item **todo, char *what) {
   *todo = xmalloc(sizeof(**todo));
	memset(*todo, 0, sizeof(**todo));
   (*todo)->what = strdup(what);
}

/* List the file(s) named in arg. */
int
com_add_todo (char *arg)
{
    if (!assert_has_value ("t", arg, "Please describe your todo item")) {
        return 1;
    }
	
	struct todo_item *todo = todo_item_list;	
	if(!todo) {
      init_todo(&todo, arg);
      todo->head = todo;
	   todo->next = todo_item_list;
      todo_item_list = todo;
   }
   else {
      init_todo(&todo->next, arg);
      todo->next->head = todo_item_list->head;
      todo_item_list = todo->next;
   }
   printf ("Added a new TODO item [ %s ]\n\n", arg);
   return 0;
}

int
com_show_todos (char *arg)
{
   if(!todo_item_list) {
       printf("You've got nothing TODO!\n");
       return 0;
   }
   struct todo_item *item, **pp = &todo_item_list->head;
   while((item = *pp) != NULL) {
       printf("* TODO: %s\n", item->what);
       pp = &item->next;
   }
   
   return 0;
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
int
com_help (char *arg)
{
   register int i;
   int printed = 0;

   for (i = 0; commands[i].name; i++)
   {
      if (!*arg || (strcmp (arg, commands[i].name) == 0))
      {
         printf ("%d) %s:\t\t%s.\n", i + 1, commands[i].name, commands[i].doc);
         printed++;
      }
   }

   if (!printed)
   {
      printf ("No commands match `%s'.  Possibilties are:\n", arg);

      for (i = 0; commands[i].name; i++)
      {
         /* Print in six columns. */
         if (printed == 6)
         {
            printed = 0;
            printf ("\n");
         }

         printf ("%s\t", commands[i].name);
         printed++;
      }

      if (printed)
         printf ("\n");
   }
   return (0);
}



/* The user wishes to quit using this program.  Just set DONE
   non-zero. */
int 
com_quit (char *arg)
{
   done = 1;
   return (0);
}
/* Return non-zero if arg is not NULL
   else print an error message and return zero. */
int
assert_has_value (char *caller, char *todo_item, char *msg)
{
   if (!todo_item || !*todo_item)
   {
      fprintf (stderr, "%s: %s\n", caller, msg);
      return (0);
   }

   return (1);
}

void *
xmalloc (size_t size)
{
   register void *value = (void*)malloc(size);
   if (value == 0)
      fprintf(stderr, "virtual memory exhausted");
   return value;
}



/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2009 Erwan Velu - All Rights Reserved
 *
 *   Permission is hereby granted, free of charge, to any person
 *   obtaining a copy of this software and associated documentation
 *   files (the "Software"), to deal in the Software without
 *   restriction, including without limitation the rights to use,
 *   copy, modify, merge, publish, distribute, sublicense, and/or
 *   sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following
 *   conditions:
 *
 *   The above copyright notice and this permission notice shall
 *   be included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 *
 * -----------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <syslinux/config.h>
#include <getkey.h>
#include "hdt-cli.h"
#include "hdt-common.h"

struct cli_mode_descr *list_modes[] = {
	&hdt_mode,
	&dmi_mode,
	&syslinux_mode,
	&pxe_mode,
	&kernel_mode,
	&cpu_mode,
	&pci_mode,
	&vesa_mode,
};

/*
 * .aliases = {"q", "quit"} won't work since it is an array of pointers, not an
 * array of variables. There is no easy way around it besides declaring the arrays of
 * strings first.
 */
char *exit_aliases[] = {"q", "quit"};
char *help_aliases[] = {"h"};

/* List of aliases */
struct cli_alias hdt_aliases[] = {
	{
		.command = CLI_EXIT,
		.nb_aliases = 2,
		.aliases = exit_aliases,
	},
	{
		.command = CLI_HELP,
		.nb_aliases = 1,
		.aliases = help_aliases,
	},
};

/**
 * set_mode - set the current mode of the cli
 * @mode:	mode to set
 *
 * Unlike cli_set_mode, this function is not used by the cli directly.
 **/
void set_mode(cli_mode_t mode, struct s_hardware* hardware)
{
	int i;

	switch (mode) {
	case EXIT_MODE:
		hdt_cli.mode = mode;
		break;
	case HDT_MODE:
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_HDT);
		break;
	case PXE_MODE:
		if (hardware->sv->filesystem != SYSLINUX_FS_PXELINUX) {
			more_printf("You are not currently using PXELINUX\n");
			break;
		}
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_PXE);
		break;
	case KERNEL_MODE:
		detect_pci(hardware);
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_KERNEL);
		break;
	case SYSLINUX_MODE:
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_SYSLINUX);
		break;
	case VESA_MODE:
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_VESA);
		break;
	case PCI_MODE:
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_PCI);
		if (!hardware->pci_detection)
			cli_detect_pci(hardware);
		break;
	case CPU_MODE:
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_CPU);
		if (!hardware->dmi_detection)
			detect_dmi(hardware);
		if (!hardware->cpu_detection)
			cpu_detect(hardware);
		break;
	case DMI_MODE:
		detect_dmi(hardware);
		if (!hardware->is_dmi_valid) {
			more_printf("No valid DMI table found, exiting.\n");
			break;
		}
		hdt_cli.mode = mode;
		snprintf(hdt_cli.prompt, sizeof(hdt_cli.prompt), "%s> ",
			 CLI_DMI);
		break;
	default:
		/* Invalid mode */
		printf("Unknown mode, please choose among:\n");
		for (i = 0; i < MAX_MODES; i++)
			printf("\t%s\n", list_modes[i]->name);
	}
}

/**
 * mode_s_to_mode_t - given a mode string, return the cli_mode_t representation
 **/
cli_mode_t mode_s_to_mode_t(char *name)
{
	int i = 0;

	for (i = 0; i < MAX_MODES; i++)
		if (!strncmp(name, list_modes[i]->name,
			     sizeof(list_modes[i]->name)))
			break;

	if (i == MAX_MODES)
		return INVALID_MODE;
	else
		return list_modes[i]->mode;
}

/**
 * find_cli_mode_descr - find the cli_mode_descr struct associated to a mode
 * @mode:	mode to look for
 * @mode_found:	store the mode if found, NULL otherwise
 *
 * Given a mode name, return a pointer to the associated cli_mode_descr
 * structure.
 * Note: the current mode name is stored in hdt_cli.mode.
 **/
void find_cli_mode_descr(cli_mode_t mode, struct cli_mode_descr **mode_found)
{
	int modes_iter = 0;

	while (modes_iter < MAX_MODES &&
	       list_modes[modes_iter]->mode != mode)
		modes_iter++;

	/* Shouldn't get here... */
	if (modes_iter == MAX_MODES)
		*mode_found = NULL;
	else
		*mode_found = list_modes[modes_iter];
}

/**
 * expand_aliases - resolve aliases mapping
 * @line:	command line to parse
 * @command:	first token in the line
 * @module:	second token in the line
 * @argc:	number of arguments
 * @argv:	array of arguments
 *
 * We maintain a small list of static alises to enhance user experience.
 * Only commands can be aliased (first token). Otherwise it can become really hairy...
 **/
static void expand_aliases(char *line __unused, char **command, char **module,
			   int *argc, char **argv)
{
	struct cli_mode_descr *mode;
	int i, j;

	find_cli_mode_descr(mode_s_to_mode_t(*command), &mode);
	if (mode != NULL && *module == NULL) {
		/*
		 * The user specified a mode instead of `set mode...', e.g.
		 * `dmi' instead of `set mode dmi'
		 */

		/* *argv is NULL since *module is NULL */
		*argc = 1;
		*argv = malloc(*argc * sizeof(char *));
		argv[0] = malloc((sizeof(*command) + 1) * sizeof(char));
		strncpy(argv[0], *command, sizeof(*command) + 1);
		dprintf("CLI DEBUG: ALIAS %s ", *command);

		strncpy(*command, CLI_SET, sizeof(CLI_SET));	/* set */

		*module = malloc(sizeof(CLI_MODE) * sizeof(char));
		strncpy(*module, CLI_MODE, sizeof(CLI_MODE));	/* mode */

		dprintf("--> %s %s %s\n", *command, *module, argv[0]);
		goto out;
	}

	/* Simple aliases mapping a single command to another one */
	for (i = 0; i < MAX_ALIASES; i++) {
		for (j = 0; j < hdt_aliases[i].nb_aliases; j++) {
			if (!strncmp(*command, hdt_aliases[i].aliases[j],
			    sizeof(hdt_aliases[i].aliases[j]))) {
				dprintf("CLI DEBUG: ALIAS %s ", *command);
				strncpy(*command, hdt_aliases[i].command,
					sizeof(hdt_aliases[i].command) + 1);
				dprintf("--> %s\n", *command);
				goto out; /* Don't allow chaining aliases */
			}
		}
	}
	return;

out:
	dprintf("CLI DEBUG: New parameters:\n");
	dprintf("CLI DEBUG: command = %s\n", *command);
	dprintf("CLI DEBUG: module  = %s\n", *module);
	dprintf("CLI DEBUG: argc    = %d\n", *argc);
	for (i = 0; i < *argc; i++)
		dprintf("CLI DEBUG: argv[%d] = %s\n", i, argv[0]);
	return;
}

/**
 * parse_command_line - low level parser for the command line
 * @line:	command line to parse
 * @command:	first token in the line
 * @module:	second token in the line
 * @argc:	number of arguments
 * @argv:	array of arguments
 *
 * The format of the command line is:
 *	<main command> [<module on which to operate> [<args>]]
 **/
static void parse_command_line(char *line, char **command, char **module,
			       int *argc, char **argv)
{
	int argc_iter, args_pos, token_found, len;
	char *pch = NULL, *pch_next = NULL;

	*command = NULL;
	*module = NULL;
	*argc = 0;

	pch = line;
	while (pch != NULL) {
		pch_next = strchr(pch + 1, ' ');

		/* End of line guaranteed to be zeroed */
		if (pch_next == NULL)
			len = (int) (strchr(pch + 1, '\0') - pch);
		else
			len = (int) (pch_next - pch);

		if (token_found == 0) {
			/* Main command to execute */
			*command = malloc((len + 1) * sizeof(char));
			strncpy(*command, pch, len);
			(*command)[len] = '\0';
			dprintf("CLI DEBUG: command = %s\n", *command);
			args_pos += len + 1;
		} else if (token_found == 1) {
			/* Module */
			*module = malloc((len + 1) * sizeof(char));
			strncpy(*module, pch, len);
			(*module)[len] = '\0';
			dprintf("CLI DEBUG: module  = %s\n", *module);
			args_pos += len + 1;
		} else
			(*argc)++;

		if (pch_next != NULL)
			pch_next++;

		token_found++;
		pch = pch_next;
	}
	dprintf("CLI DEBUG: argc    = %d\n", *argc);

	/* Skip arguments handling if none is supplied */
	if (!*argc)
		return;

	/* Transform the arguments string into an array */
	*argv = malloc(*argc * sizeof(char *));
	pch = strtok(line + args_pos, CLI_SPACE);
	while (pch != NULL) {
		dprintf("CLI DEBUG: argv[%d] = %s\n", argc_iter, pch);
		argv[argc_iter] = malloc(sizeof(pch) * sizeof(char));
		strncpy(argv[argc_iter], pch, sizeof(pch));
		argc_iter++;
		pch = strtok(NULL, CLI_SPACE);
	}
}

/**
 * find_cli_callback_descr - find a callback in a list of modules
 * @module_name:	Name of the module to find
 * @modules_list:	Lits of modules among which to find @module_name
 * @module_found:	Pointer to the matched module, NULL if not found
 *
 * Given a module name and a list of possible modules, find the corresponding
 * module structure that matches the module name and store it in @module_found.
 **/
void find_cli_callback_descr(const char* module_name,
			     struct cli_module_descr* modules_list,
			     struct cli_callback_descr** module_found)
{
	int modules_iter = 0;
	int module_len = strlen(module_name);

	if (modules_list == NULL)
		goto not_found;

	/* Find the callback to execute */
	while (modules_iter < modules_list->nb_modules
	       && strncmp(module_name,
			  modules_list->modules[modules_iter].name,
			  module_len) != 0)
		modules_iter++;

	if (modules_iter != modules_list->nb_modules) {
		*module_found = &(modules_list->modules[modules_iter]);
		dprintf("CLI DEBUG: module %s found\n", (*module_found)->name);
		return;
	}

not_found:
	*module_found = NULL;
	return;
}

/**
 * exec_command - main logic to map the command line to callbacks
 **/
static void exec_command(char *line,
			 struct s_hardware *hardware)
{
	int argc, i = 0;
	char *command = NULL, *module = NULL;
	char **argv = NULL;
	struct cli_callback_descr* current_module = NULL;
	struct cli_mode_descr *current_mode;

	/* Find the mode selected */
	find_cli_mode_descr(hdt_cli.mode, &current_mode);
	if (current_mode == NULL) {
		/* Shouldn't get here... */
		printf("!!! BUG: Mode '%s' unknown.\n", hdt_cli.mode);
		return;
	}

	/* This will allocate memory that will need to be freed */
	parse_command_line(line, &command, &module, &argc, argv);

	/* Expand shortcuts, if needed */
	expand_aliases(line, &command, &module, &argc, argv);

	if (module == NULL) {
		dprintf("CLI DEBUG: single command detected\n", CLI_SHOW);
		/*
		 * A single word was specified: look at the list of default
		 * commands in the current mode to see if there is a match.
		 * If not, it may be a generic function (exit, help, ...). These
		 * are stored in the list of default commands of the hdt mode.
		 */
		find_cli_callback_descr(command, current_mode->default_modules,
				        &current_module);
		if (current_module != NULL) {
			current_module->exec(argc, argv, hardware);
			return;
		}
		else {
			find_cli_callback_descr(command, hdt_mode.default_modules,
					        &current_module);
			if (current_module != NULL) {
				current_module->exec(argc, argv, hardware);
				return;
			}
		}
	}

	/*
	 * A module has been specified! We now need to find the type of command.
	 *
	 * The syntax of the cli is the following:
	 *    <type of command> <module on which to operate> <args>
	 * e.g.
	 *    dmi> show system
	 *    dmi> show bank 1
	 *    dmi> show memory 0 1
	 *    pci> show device 12
	 *    hdt> set mode dmi
	 */
	if (!strncmp(command, CLI_SHOW, sizeof(CLI_SHOW) - 1)) {
		dprintf("CLI DEBUG: %s command detected\n", CLI_SHOW);
		find_cli_callback_descr(module, current_mode->show_modules,
					&current_module);
		/* Execute the callback */
		if (current_module != NULL)
			return current_module->exec(argc, argv, hardware);
		else {
			if (current_mode->show_modules != NULL &&
			    current_mode->show_modules->default_callback != NULL)
				return current_mode->show_modules
						   ->default_callback(argc,
								      argv,
								      hardware);
		}
	} else if (!strncmp(command, CLI_SET, sizeof(CLI_SET) - 1)) {
		dprintf("CLI DEBUG: %s command detected\n", CLI_SET);
		find_cli_callback_descr(module, current_mode->set_modules,
					&current_module);
		/* Execute the callback */
		if (current_module != NULL)
			return current_module->exec(argc, argv, hardware);
		else {
			if (current_mode->set_modules != NULL &&
			    current_mode->set_modules->default_callback != NULL)
				return current_mode->set_modules
						   ->default_callback(argc,
								      argv,
								      hardware);
		}
	}
	dprintf("CLI DEBUG: callback not found!\n", argc);

	/* Let's not forget to clean ourselves */
	free(command);
	free(module);
	for (i = 0; i < argc; i++)
		free(argv[i]);
	free(argv);
}

static void reset_prompt()
{
	/* No need to display the prompt if we exit */
	if (hdt_cli.mode != EXIT_MODE) {
		printf("%s", hdt_cli.prompt);
		/* Reset the line */
		memset(hdt_cli.input, '\0', MAX_LINE_SIZE);
		hdt_cli.cursor_pos = 0;
	}
}

/* Code that manages the cli mode */
void start_cli_mode(struct s_hardware *hardware)
{
	int current_key = 0;
	int future_history_pos=1; /* Temp variable*/
	bool display_history=true; /* Temp Variable*/
	char temp_command[MAX_LINE_SIZE];

	hdt_cli.cursor_pos=0;
	memset(hdt_cli.input, '\0', MAX_LINE_SIZE);
	memset(hdt_cli.history, '\0', sizeof(hdt_cli.history));
	hdt_cli.history_pos=1;
	hdt_cli.max_history_pos=1;

	set_mode(HDT_MODE, hardware);

	printf("Entering CLI mode\n");

	/* Display the cursor */
	fputs("\033[?25h", stdout);

	reset_prompt();

	while (hdt_cli.mode != EXIT_MODE) {

		//fgets(cli_line, sizeof cli_line, stdin);
		current_key = get_key(stdin, 0);

		switch (current_key) {
			/* clear until then end of line */
		case KEY_CTRL('k'):
			/* Clear the end of the line */
			fputs("\033[0K", stdout);
			memset(&hdt_cli.input[hdt_cli.cursor_pos], 0,
			       strlen(hdt_cli.input) - hdt_cli.cursor_pos);
			break;

		case KEY_CTRL('c'):
			more_printf("\n");
			reset_prompt();
			break;

		case KEY_TAB:
			break;

		case KEY_LEFT:
			if (hdt_cli.cursor_pos > 0) {
				fputs("\033[1D", stdout);
				hdt_cli.cursor_pos--;
			}
			break;

		case KEY_RIGHT:
			if (hdt_cli.cursor_pos < (int)strlen(hdt_cli.input)) {
				fputs("\033[1C", stdout);
				hdt_cli.cursor_pos++;
			}
			break;

		case KEY_CTRL('e'):
		case KEY_END:
			/* Calling with a 0 value will make the cursor move */
			/* So, let's move the cursor only if needed */
			if ((strlen(hdt_cli.input) - hdt_cli.cursor_pos) > 0) {
				memset(temp_command, 0, sizeof(temp_command));
				sprintf(temp_command, "\033[%dC",
					strlen(hdt_cli.input) - hdt_cli.cursor_pos);
				/* Return to the begining of line */
				fputs(temp_command, stdout);
				hdt_cli.cursor_pos = strlen(hdt_cli.input);
			}
			break;

		case KEY_CTRL('a'):
		case KEY_HOME:
			/* Calling with a 0 value will make the cursor move */
			/* So, let's move the cursor only if needed */
			if (hdt_cli.cursor_pos > 0) {
				memset(temp_command, 0, sizeof(temp_command));
				sprintf(temp_command, "\033[%dD",
					hdt_cli.cursor_pos);
				/* Return to the begining of line */
				fputs(temp_command, stdout);
				hdt_cli.cursor_pos = 0;
			}
			break;

		case KEY_UP:
			/* We have to compute the next position*/
			future_history_pos=hdt_cli.history_pos;
			if (future_history_pos==1) {
				future_history_pos=MAX_HISTORY_SIZE-1;
			} else {
				future_history_pos--;
			}
			/* Does the next position is valid */
			if (strlen(hdt_cli.history[future_history_pos])==0) break;

			/* Let's make that future position the one we use*/
			hdt_cli.history_pos=future_history_pos;

			/* Clear the line */
			fputs("\033[2K", stdout);

			/* Move to the begining of line*/
			fputs("\033[0G", stdout);

			reset_prompt();
			printf("%s",hdt_cli.history[hdt_cli.history_pos]);
			strncpy(hdt_cli.input,hdt_cli.history[hdt_cli.history_pos],sizeof(hdt_cli.input));
			hdt_cli.cursor_pos=strlen(hdt_cli.input);
			break;

		case KEY_DOWN:
			display_history=true;

			/* We have to compute the next position*/
			future_history_pos=hdt_cli.history_pos;
			if (future_history_pos==MAX_HISTORY_SIZE-1) {
				future_history_pos=1;
			} else {
				future_history_pos++;
			}
			/* Does the next position is valid */
			if (strlen(hdt_cli.history[future_history_pos])==0) display_history = false;

			/* An exception is made to reach the last empty line */
			if (future_history_pos==hdt_cli.max_history_pos) display_history=true;
			if (display_history==false) break;

			/* Let's make that future position the one we use*/
			hdt_cli.history_pos=future_history_pos;

			/* Clear the line */
			fputs("\033[2K", stdout);

			/* Move to the begining of line*/
			fputs("\033[0G", stdout);

			reset_prompt();
			printf("%s",hdt_cli.history[hdt_cli.history_pos]);
			strncpy(hdt_cli.input,hdt_cli.history[hdt_cli.history_pos],sizeof(hdt_cli.input));
			hdt_cli.cursor_pos=strlen(hdt_cli.input);
			break;

		case KEY_ENTER:
			more_printf("\n");
			if (hdt_cli.history_pos == MAX_HISTORY_SIZE-1) hdt_cli.history_pos=1;
			strncpy(hdt_cli.history[hdt_cli.history_pos],skipspace(hdt_cli.input),sizeof(hdt_cli.history[hdt_cli.history_pos]));
			hdt_cli.history_pos++;
			if (hdt_cli.history_pos>hdt_cli.max_history_pos) hdt_cli.max_history_pos=hdt_cli.history_pos;
			exec_command(skipspace(hdt_cli.input), hardware);
			reset_prompt();
			break;

		case KEY_BACKSPACE:
			/* Don't delete prompt */
			if (hdt_cli.cursor_pos == 0)
				break;

			for (int c = hdt_cli.cursor_pos - 1;
			     c < (int)strlen(hdt_cli.input) - 1; c++)
				hdt_cli.input[c] = hdt_cli.input[c + 1];
			hdt_cli.input[strlen(hdt_cli.input) - 1] = '\0';

			/* Get one char back */
			fputs("\033[1D", stdout);
			/* Clear the end of the line */
			fputs("\033[0K", stdout);

			/* Print the resulting buffer */
			printf("%s", hdt_cli.input + hdt_cli.cursor_pos - 1);

			/* Realing to the place we were */
			memset(temp_command, 0, sizeof(temp_command));
			sprintf(temp_command, "\033[%dD",
				strlen(hdt_cli.input + hdt_cli.cursor_pos - 1));
			fputs(temp_command, stdout);
			fputs("\033[1C", stdout);
			/* Don't decrement the position unless
			 * if we are at then end of the line*/
			if (hdt_cli.cursor_pos > (int)strlen(hdt_cli.input))
				hdt_cli.cursor_pos--;
			break;

		case KEY_F1:
			more_printf("\n");
			exec_command(CLI_HELP, hardware);
			reset_prompt();
			break;

		default:
			if ( ( current_key < 0x20 ) || ( current_key > 0x7e ) ) break;
			/* Prevent overflow */
			if (hdt_cli.cursor_pos > MAX_LINE_SIZE - 2)
				break;
			/* If we aren't at the end of the input line, let's insert */
			if (hdt_cli.cursor_pos < (int)strlen(hdt_cli.input)) {
				char key[2];
				int trailing_chars =
				    strlen(hdt_cli.input) - hdt_cli.cursor_pos;
				memset(temp_command, 0, sizeof(temp_command));
				strncpy(temp_command, hdt_cli.input,
					hdt_cli.cursor_pos);
				sprintf(key, "%c", current_key);
				strncat(temp_command, key, 1);
				strncat(temp_command,
					hdt_cli.input + hdt_cli.cursor_pos,
					trailing_chars);
				memset(hdt_cli.input, 0, sizeof(hdt_cli.input));
				snprintf(hdt_cli.input, sizeof(hdt_cli.input), "%s",
					 temp_command);

				/* Clear the end of the line */
				fputs("\033[0K", stdout);

				/* Print the resulting buffer */
				printf("%s", hdt_cli.input + hdt_cli.cursor_pos);
				sprintf(temp_command, "\033[%dD",
					trailing_chars);
				/* Return where we must put the new char */
				fputs(temp_command, stdout);

			} else {
				putchar(current_key);
				hdt_cli.input[hdt_cli.cursor_pos] = current_key;
			}
			hdt_cli.cursor_pos++;
			break;
		}
	}
}
